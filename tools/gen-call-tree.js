#!/usr/bin/env node

const fs = require("fs");
const path = require("path");

const DEFAULT_INPUTS = ["*.c"];
const DEFAULT_OUTPUT = "call-tree.txt";
const CONTROL_WORDS = new Set([
	"if",
	"for",
	"while",
	"switch",
	"return",
	"sizeof",
	"_Alignof",
	"alignof",
	"typeof",
	"case",
	"do"
]);

function normalizePath(filePath) {
	// Convert paths to stable slash-separated relative names for the report.
	return path.relative(process.cwd(), filePath).replace(/\\/g, "/");
}

function parseArgs(argv) {
	// Initialize defaults and collect positional source files.
	const config = {
		inputs: [],
		output: DEFAULT_OUTPUT,
		followIncludes: true,
		help: false
	};

	// Walk the command line one token at a time.
	for (let i = 0; i < argv.length; ++i) {
		const arg = argv[i];

		// Recognize help flags before any path handling.
		if (arg === "--help" || arg === "-h") {
			config.help = true;
			continue;
		}

		// Allow callers to scan only files named on the command line.
		if (arg === "--no-includes") {
			config.followIncludes = false;
			continue;
		}

		// Read the output path from the next token.
		if (arg === "--out" || arg === "-o") {
			if (i + 1 >= argv.length)
				throw new Error(`${arg} needs a path`);
			config.output = argv[++i];
			continue;
		}

		// Store any remaining argument as an input file.
		config.inputs.push(arg);
	}

	// Use current-directory C sources when no explicit files were provided.
	if (config.inputs.length === 0)
		config.inputs = DEFAULT_INPUTS.slice();

	return config;
}

function hasGlob(pattern) {
	// Detect simple shell-style wildcards handled by this script.
	return /[*?]/.test(pattern);
}

function escapeRegex(text) {
	// Escape characters that have special meaning in regular expressions.
	return text.replace(/[|\\{}()[\]^$+?.]/g, "\\$&");
}

function globToRegex(pattern) {
	// Convert a slash-separated glob into a regular expression.
	const normalized = pattern.replace(/\\/g, "/");
	let regex = "^";

	// Translate each wildcard while keeping normal characters literal.
	for (let i = 0; i < normalized.length; ++i) {
		const c = normalized[i];
		if (c === "*") {
			if (normalized[i + 1] === "*") {
				regex += ".*";
				++i;
			} else {
				regex += "[^/]*";
			}
		} else if (c === "?") {
			regex += "[^/]";
		} else {
			regex += escapeRegex(c);
		}
	}

	return new RegExp(regex + "$");
}

function globBaseDir(pattern) {
	// Find the non-wildcard directory prefix of a glob pattern.
	const normalized = pattern.replace(/\\/g, "/");
	const parts = normalized.split("/");
	const firstGlob = parts.findIndex(part => hasGlob(part));
	if (firstGlob < 0)
		return path.dirname(pattern) || ".";

	const baseParts = parts.slice(0, firstGlob);
	return baseParts.length ? baseParts.join("/") : ".";
}

function walkFiles(dirName) {
	// Recursively collect files below a directory.
	const files = [];
	if (!fs.existsSync(dirName))
		return files;

	// Visit directory entries in stable lexical order.
	for (const entry of fs.readdirSync(dirName, { withFileTypes: true }).sort((a, b) => a.name.localeCompare(b.name))) {
		const fullPath = path.join(dirName, entry.name);
		if (entry.isDirectory())
			files.push(...walkFiles(fullPath));
		else if (entry.isFile())
			files.push(normalizePath(path.resolve(fullPath)));
	}

	return files;
}

function expandGlob(pattern) {
	// Match one glob pattern against files below its literal base directory.
	const normalizedPattern = pattern.replace(/\\/g, "/");
	const matcher = globToRegex(normalizedPattern);
	const baseDir = path.resolve(globBaseDir(normalizedPattern));
	const matches = [];

	// Test candidate paths relative to the current working directory.
	for (const fileName of walkFiles(baseDir)) {
		if (matcher.test(fileName))
			matches.push(fileName);
	}

	return matches;
}

function expandInputPatterns(inputs) {
	// Expand wildcard inputs and keep explicit files in caller order.
	const expanded = [];
	const seen = new Set();

	// Append each match once, preserving the order of the input patterns.
	for (const input of inputs) {
		const matches = hasGlob(input) ? expandGlob(input) : [normalizePath(path.resolve(input))];
		if (matches.length === 0)
			throw new Error(`No files matched ${input}`);

		for (const match of matches) {
			if (!seen.has(match)) {
				seen.add(match);
				expanded.push(match);
			}
		}
	}

	return expanded;
}

function stripCommentsOnly(source) {
	// Remove comments while preserving strings and newlines for directives.
	let output = "";
	let i = 0;

	// Scan through comments, strings, and ordinary text.
	while (i < source.length) {
		const c = source[i];
		const n = source[i + 1];

		// Blank line comments while preserving the newline.
		if (c === "/" && n === "/") {
			output += "  ";
			i += 2;
			while (i < source.length && source[i] !== "\n") {
				output += " ";
				++i;
			}
			continue;
		}

		// Blank block comments while preserving embedded newlines.
		if (c === "/" && n === "*") {
			output += "  ";
			i += 2;
			while (i < source.length && !(source[i] === "*" && source[i + 1] === "/")) {
				output += source[i] === "\n" ? "\n" : " ";
				++i;
			}
			if (i < source.length) {
				output += "  ";
				i += 2;
			}
			continue;
		}

		// Copy string and character literals so include paths remain readable.
		if (c === "\"" || c === "'") {
			const quote = c;
			output += c;
			++i;
			while (i < source.length) {
				output += source[i];
				if (source[i] === "\\") {
					++i;
					if (i < source.length)
						output += source[i];
				} else if (source[i] === quote) {
					++i;
					break;
				}
				++i;
			}
			continue;
		}

		// Copy ordinary source text unchanged.
		output += c;
		++i;
	}

	return output;
}

function collectLocalIncludes(fileName, source) {
	// Collect quoted include paths from a source file.
	const includes = [];
	const clean = stripCommentsOnly(source);
	const dirName = path.dirname(fileName);
	const includePattern = /^\s*#\s*include\s*"([^"]+)"/gm;
	let match;

	// Resolve each include relative to the file that contains it.
	while ((match = includePattern.exec(clean))) {
		const includePath = path.resolve(dirName, match[1]);
		includes.push(normalizePath(includePath));
	}

	return includes;
}

function expandIncludedFiles(inputFiles) {
	// Recursively append local quoted includes in translation-unit order.
	const expanded = [];
	const seen = new Set();
	const missing = [];

	function visit(fileName, includedFrom) {
		// Normalize and dedupe each visited file.
		const normalized = normalizePath(path.resolve(fileName));
		if (seen.has(normalized))
			return;
		seen.add(normalized);

		// Read the file, reporting missing nested includes without stopping.
		let source;
		try {
			source = readSource(normalized);
		} catch (error) {
			if (includedFrom) {
				missing.push(`${normalized} included from ${includedFrom}`);
				return;
			}
			throw error;
		}

		// Add the file before its includes to match textual include order.
		expanded.push(normalized);
		for (const includePath of collectLocalIncludes(normalized, source))
			visit(includePath, normalized);
	}

	// Visit each initial input and collect warnings for unresolved includes.
	for (const fileName of inputFiles)
		visit(fileName, null);

	return { files: expanded, missing };
}

function printHelp() {
	// Print the supported invocation forms.
	console.log(`Usage:
  node tools/gen-call-tree.js [source.c ...] [--out output.txt] [--no-includes]
  node tools/gen-call-tree.js "*.c" --out call-tree.txt

Defaults:
  inputs: ${DEFAULT_INPUTS.join(", ")}
  output: ${DEFAULT_OUTPUT}

The generated tree follows direct and inferred dependencies between functions
defined in the input files. Function-like macros that expand to in-scope
function calls are followed for root/callee purposes. Bare in-scope function
references and parsed top-level function-pointer tables are followed as
indirect edges. Calls through opaque function-pointer variables are omitted
when the target cannot be inferred. Quoted local includes are followed
recursively by default. Glob patterns support *, ?, and **.`);
}

function readSource(filePath) {
	// Load each source as UTF-8 text for static scanning.
	return fs.readFileSync(filePath, "utf8");
}

function stripCommentsAndLiterals(source) {
	// Preserve character positions while blanking comments and literal contents.
	let output = "";
	let i = 0;

	// Scan the source with a small lexical state machine.
	while (i < source.length) {
		const c = source[i];
		const n = source[i + 1];

		// Blank line comments while keeping the newline.
		if (c === "/" && n === "/") {
			output += "  ";
			i += 2;
			while (i < source.length && source[i] !== "\n") {
				output += " ";
				++i;
			}
			continue;
		}

		// Blank block comments while preserving embedded newlines.
		if (c === "/" && n === "*") {
			output += "  ";
			i += 2;
			while (i < source.length && !(source[i] === "*" && source[i + 1] === "/")) {
				output += source[i] === "\n" ? "\n" : " ";
				++i;
			}
			if (i < source.length) {
				output += "  ";
				i += 2;
			}
			continue;
		}

		// Blank string and character literal contents while preserving quotes.
		if (c === "\"" || c === "'") {
			const quote = c;
			output += quote;
			++i;
			while (i < source.length) {
				if (source[i] === "\\") {
					output += " ";
					++i;
					if (i < source.length) {
						output += source[i] === "\n" ? "\n" : " ";
						++i;
					}
					continue;
				}
				if (source[i] === quote) {
					output += quote;
					++i;
					break;
				}
				output += source[i] === "\n" ? "\n" : " ";
				++i;
			}
			continue;
		}

		// Copy normal source characters unchanged.
		output += c;
		++i;
	}

	return output;
}

function makeLineStarts(source) {
	// Record the byte index where each 1-based line begins.
	const starts = [0];

	// Collect starts by scanning for line feeds.
	for (let i = 0; i < source.length; ++i) {
		if (source.charCodeAt(i) === 10)
			starts.push(i + 1);
	}

	return starts;
}

function lineAt(lineStarts, index) {
	// Locate a source index with binary search over line starts.
	let lo = 0;
	let hi = lineStarts.length - 1;

	// Narrow the search until hi is the matching 0-based line index.
	while (lo <= hi) {
		const mid = (lo + hi) >> 1;
		if (lineStarts[mid] <= index)
			lo = mid + 1;
		else
			hi = mid - 1;
	}

	return hi + 1;
}

function findMatchingBrace(source, openIndex) {
	// Balance braces starting at an opening brace index.
	let depth = 0;

	// Walk forward until the original brace closes.
	for (let i = openIndex; i < source.length; ++i) {
		if (source[i] === "{")
			++depth;
		else if (source[i] === "}") {
			--depth;
			if (depth === 0)
				return i;
		}
	}

	return -1;
}

function buildAliasByLine(source) {
	// Track simple object-like preprocessor aliases as they become active.
	const lines = source.split(/\n/);
	const active = new Map();
	const byLine = Array(lines.length + 2);

	// Snapshot active aliases before processing each source line.
	for (let i = 0; i < lines.length; ++i) {
		byLine[i + 1] = new Map(active);
		const trimmed = lines[i].replace(/\/\/.*$/, "").trim();
		const undef = trimmed.match(/^#\s*undef\s+([A-Za-z_]\w*)\b/);
		if (undef) {
			active.delete(undef[1]);
			continue;
		}
		const define = trimmed.match(/^#\s*define\s+([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*$/);
		if (define)
			active.set(define[1], define[2]);
	}

	// Provide a final snapshot for indexes at end-of-file.
	byLine[lines.length + 1] = new Map(active);
	return byLine;
}

function collectFunctionMacros(source, fileName) {
	// Collect function-like macro bodies so their in-scope helper calls count.
	const lines = source.split(/\n/);
	const macros = new Map();

	// Join backslash-continued defines and store their stripped bodies.
	for (let i = 0; i < lines.length; ++i) {
		let line = lines[i];
		if (!/^\s*#\s*define\s+[A-Za-z_]\w*\s*\(/.test(line))
			continue;

		const startLine = i + 1;
		let text = line;
		while (/\\\s*$/.test(text) && i + 1 < lines.length) {
			text = text.replace(/\\\s*$/, " ");
			text += lines[++i];
		}

		const match = text.match(/^\s*#\s*define\s+([A-Za-z_]\w*)\s*\(([^)]*)\)\s*([\s\S]*)$/);
		if (match) {
			macros.set(match[1], {
				name: match[1],
				fileName,
				line: startLine,
				body: stripCommentsAndLiterals(match[3])
			});
		}
	}

	return macros;
}

function findFunctions(fileName, source, aliasesByLine) {
	// Strip non-code text while keeping indexes aligned with the original file.
	const clean = stripCommentsAndLiterals(source);
	const lineStarts = makeLineStarts(clean);
	const functions = [];
	const definitionPattern = /\b([A-Za-z_][A-Za-z0-9_]*)\s*\(([^;{}()]|\([^;{}()]*\))*\)\s*\{/g;
	let match;

	// Find function-like constructs and filter down to real definitions.
	while ((match = definitionPattern.exec(clean))) {
		const tokenName = match[1];
		if (CONTROL_WORDS.has(tokenName) || tokenName === "__attribute__" || tokenName === "__declspec")
			continue;

		const nameIndex = match.index;
		const line = lineAt(lineStarts, nameIndex);
		const braceIndex = clean.indexOf("{", definitionPattern.lastIndex - 1);
		const closeIndex = findMatchingBrace(clean, braceIndex);
		if (closeIndex < 0)
			continue;

		const prefixStart = Math.max(0, nameIndex - 300);
		const prefix = clean.slice(prefixStart, nameIndex);
		const lastTerminator = Math.max(prefix.lastIndexOf(";"), prefix.lastIndexOf("}"), prefix.lastIndexOf("{"), prefix.lastIndexOf("#"));
		const definitionStart = prefixStart + lastTerminator + 1;
		const signaturePrefix = prefix.slice(lastTerminator + 1).trim();
		if (!signaturePrefix || signaturePrefix.includes("=") || /\btypedef\b/.test(signaturePrefix))
			continue;
		if (/\b(?:if|for|while|switch|return)\s*$/.test(signaturePrefix))
			continue;

		const aliases = aliasesByLine[line] || new Map();
		const name = aliases.get(tokenName) || tokenName;
		functions.push({
			fileName,
			name,
			tokenName,
			line,
			start: definitionStart,
			end: closeIndex + 1,
			body: clean.slice(braceIndex + 1, closeIndex),
			bodyStart: braceIndex + 1,
			lineStarts
		});
		definitionPattern.lastIndex = closeIndex + 1;
	}

	return functions;
}

function previousNonSpace(text, index) {
	// Find the previous non-whitespace character before a token.
	for (let i = index - 1; i >= 0; --i) {
		if (!/\s/.test(text[i]))
			return text[i];
	}

	return "";
}

function nextNonSpace(text, index) {
	// Find the next non-whitespace character after a token.
	for (let i = index; i < text.length; ++i) {
		if (!/\s/.test(text[i]))
			return text[i];
	}

	return "";
}

function rawCallTokens(text) {
	// Find simple identifier-followed-by-parenthesis call sites.
	const calls = [];
	const callPattern = /\b([A-Za-z_][A-Za-z0-9_]*)\s*\(/g;
	let match;

	// Filter out control words and member-call spellings.
	while ((match = callPattern.exec(text))) {
		const name = match[1];
		if (CONTROL_WORDS.has(name))
			continue;

		const previous = previousNonSpace(text, match.index);
		if (previous === "." || previous === ">" || previous === "#")
			continue;

		calls.push({ name, index: match.index });
	}

	return calls;
}

function rawIdentifierTokens(text) {
	// Find ordinary identifier references, including non-call function pointers.
	const identifiers = [];
	const identifierPattern = /\b([A-Za-z_][A-Za-z0-9_]*)\b/g;
	let match;

	// Filter out control words and member-access spellings.
	while ((match = identifierPattern.exec(text))) {
		const name = match[1];
		if (CONTROL_WORDS.has(name))
			continue;

		const previous = previousNonSpace(text, match.index);
		if (previous === "." || previous === ">" || previous === "#")
			continue;

		identifiers.push({
			name,
			index: match.index,
			isCall: nextNonSpace(text, match.index + name.length) === "("
		});
	}

	return identifiers;
}

function collectFunctionPointerTables(fileName, source, definitions, internalNames, aliasesByLine) {
	// Collect top-level initializer tables that store in-scope function names.
	const clean = stripCommentsAndLiterals(source);
	const lineStarts = makeLineStarts(clean);
	const tables = [];
	const ranges = definitions
		.filter(def => def.fileName === fileName)
		.map(def => ({ start: def.start, end: def.end }))
		.sort((a, b) => a.start - b.start);

	let rangeIndex = 0;
	let statementStart = 0;
	let braceDepth = 0;

	// Scan only top-level statements, skipping function bodies entirely.
	for (let i = 0; i < clean.length; ++i) {
		if (rangeIndex < ranges.length && i === ranges[rangeIndex].start) {
			i = ranges[rangeIndex].end - 1;
			statementStart = ranges[rangeIndex].end;
			braceDepth = 0;
			++rangeIndex;
			continue;
		}

		const c = clean[i];
		if (c === "{") {
			++braceDepth;
			continue;
		}
		if (c === "}") {
			braceDepth = Math.max(0, braceDepth - 1);
			continue;
		}
		if (c !== ";" || braceDepth !== 0)
			continue;

		// Treat brace initializers with internal function names as pointer tables.
		const statement = clean.slice(statementStart, i + 1);
		statementStart = i + 1;
		const equalsIndex = statement.indexOf("=");
		if (equalsIndex < 0 || !statement.includes("{"))
			continue;

		const beforeEquals = statement.slice(0, equalsIndex);
		const nameMatch = beforeEquals.match(/\b([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\])?\s*$/);
		if (!nameMatch)
			continue;

		const refs = new Set();
		const initializer = statement.slice(equalsIndex + 1);
		const initializerStart = statementStart - statement.length + equalsIndex + 1;
		for (const identifier of rawIdentifierTokens(initializer)) {
			const line = lineAt(lineStarts, initializerStart + identifier.index);
			const aliasMap = aliasesByLine[line] || new Map();
			const mapped = aliasMap.get(identifier.name) || identifier.name;
			if (internalNames.has(mapped))
				refs.add(mapped);
		}

		if (refs.size) {
			const name = nameMatch[1];
			const nameIndex = statementStart - statement.length + beforeEquals.lastIndexOf(name);
			tables.push({
				fileName,
				name,
				line: lineAt(lineStarts, nameIndex),
				refs
			});
		}
	}

	return tables;
}

function firstDefinitionLine(groups, name) {
	// Return the earliest definition line for sorting.
	return groups.get(name).defs.reduce((min, def) => Math.min(min, def.line), Infinity);
}

function firstDefinitionFile(groups, name) {
	// Return the earliest definition file for sorting.
	return groups.get(name).defs.slice().sort((a, b) => a.line - b.line)[0].fileName;
}

function locationOf(def) {
	// Render a definition location, including the pre-alias token when useful.
	const aliasNote = def.tokenName !== def.name ? ` token ${def.tokenName}` : "";
	return `${def.fileName}:${def.line}${aliasNote}`;
}

function labelFor(groups, name) {
	// Render a function label with one or more source locations.
	const group = groups.get(name);
	if (group.defs.length === 1)
		return `${name} [${locationOf(group.defs[0])}]`;

	return `${name} [${group.defs.length} defs: ${group.defs.map(locationOf).join("; ")}]`;
}

function tableLabel(table) {
	// Render a function-pointer table location.
	return `${table.name} [${table.fileName}:${table.line}]`;
}

function buildGraph(inputFiles) {
	// Read all inputs and prepare macro state for each source.
	const sourceByFile = new Map();
	const aliasesByFile = new Map();
	const functionMacros = new Map();

	// Load source files and collect preprocessor information.
	for (const fileName of inputFiles) {
		const source = readSource(fileName);
		sourceByFile.set(fileName, source);
		aliasesByFile.set(fileName, buildAliasByLine(source));
		for (const [name, macro] of collectFunctionMacros(source, fileName))
			functionMacros.set(name, macro);
	}

	// Parse definitions from all inputs.
	const defs = [];
	for (const [fileName, source] of sourceByFile)
		defs.push(...findFunctions(fileName, source, aliasesByFile.get(fileName)));

	// Group conditional or macro-renamed definitions by logical function name.
	const groups = new Map();
	for (const def of defs) {
		if (!groups.has(def.name))
			groups.set(def.name, {
				name: def.name,
				defs: [],
				calls: new Set(),
				directCalls: new Set(),
				macroCalls: new Set(),
				referenceCalls: new Set(),
				tableCalls: new Map()
			});
		groups.get(def.name).defs.push(def);
	}

	const internalNames = new Set(groups.keys());
	const functionPointerTables = [];
	for (const [fileName, source] of sourceByFile)
		functionPointerTables.push(...collectFunctionPointerTables(fileName, source, defs, internalNames, aliasesByFile.get(fileName)));

	const functionPointerTablesByName = new Map();
	for (const table of functionPointerTables) {
		if (!functionPointerTablesByName.has(table.name))
			functionPointerTablesByName.set(table.name, []);
		functionPointerTablesByName.get(table.name).push(table);
	}

	function addCall(group, callee, kind, tableName) {
		// Record one dependency edge in the union set and its specific edge kind.
		if (!internalNames.has(callee))
			return;

		group.calls.add(callee);
		if (kind === "direct")
			group.directCalls.add(callee);
		else if (kind === "macro")
			group.macroCalls.add(callee);
		else if (kind === "reference")
			group.referenceCalls.add(callee);
		else if (kind === "table") {
			if (!group.tableCalls.has(callee))
				group.tableCalls.set(callee, new Set());
			group.tableCalls.get(callee).add(tableName);
		}
	}

	function resolveMacroCalls(name, aliasMap, seen = new Set()) {
		// Resolve simple macro calls recursively into in-scope function calls.
		const actualName = aliasMap && aliasMap.get(name) ? aliasMap.get(name) : name;
		const macro = functionMacros.get(actualName) || functionMacros.get(name);
		const result = new Set();
		if (!macro || seen.has(actualName))
			return result;

		seen.add(actualName);
		for (const call of rawCallTokens(macro.body)) {
			const mapped = aliasMap && aliasMap.get(call.name) ? aliasMap.get(call.name) : call.name;
			if (internalNames.has(mapped))
				result.add(mapped);
			if (functionMacros.has(mapped) || functionMacros.has(call.name)) {
				for (const dep of resolveMacroCalls(mapped, aliasMap, seen))
					result.add(dep);
			}
		}

		return result;
	}

	// Collect direct in-scope call edges from each function body.
	for (const group of groups.values()) {
		for (const def of group.defs) {
			const aliasesByLine = aliasesByFile.get(def.fileName);
			for (const call of rawCallTokens(def.body)) {
				const line = lineAt(def.lineStarts, def.bodyStart + call.index);
				const aliasMap = aliasesByLine[line] || new Map();
				const mapped = aliasMap.get(call.name) || call.name;
				if (internalNames.has(mapped)) {
					addCall(group, mapped, "direct");
					continue;
				}
				if (functionMacros.has(mapped) || functionMacros.has(call.name)) {
					for (const dep of resolveMacroCalls(mapped, aliasMap))
						addCall(group, dep, "macro");
				}
			}
		}
		group.calls.delete(group.name);
	}

	// Re-add explicit self-recursive calls while avoiding macro self-expansion.
	for (const group of groups.values()) {
		for (const def of group.defs) {
			const aliasesByLine = aliasesByFile.get(def.fileName);
			for (const call of rawCallTokens(def.body)) {
				const line = lineAt(def.lineStarts, def.bodyStart + call.index);
				const aliasMap = aliasesByLine[line] || new Map();
				const mapped = aliasMap.get(call.name) || call.name;
				if (mapped === group.name)
					addCall(group, mapped, "direct");
			}
		}
	}

	// Collect non-call function references and referenced function pointer tables.
	for (const group of groups.values()) {
		for (const def of group.defs) {
			const aliasesByLine = aliasesByFile.get(def.fileName);
			for (const identifier of rawIdentifierTokens(def.body)) {
				const line = lineAt(def.lineStarts, def.bodyStart + identifier.index);
				const aliasMap = aliasesByLine[line] || new Map();
				const mapped = aliasMap.get(identifier.name) || identifier.name;

				// Treat a bare in-scope function name as an indirect function reference.
				if (!identifier.isCall && mapped !== group.name && internalNames.has(mapped))
					addCall(group, mapped, "reference");

				// Treat references to function-pointer tables as edges to their stored functions.
				const tables = functionPointerTablesByName.get(mapped);
				if (tables) {
					for (const table of tables) {
						for (const callee of table.refs) {
							if (callee !== group.name)
								addCall(group, callee, "table", table.name);
						}
					}
				}
			}
		}
	}

	// Build incoming edge sets for root detection.
	const incoming = new Map([...internalNames].map(name => [name, new Set()]));
	for (const [name, group] of groups) {
		for (const callee of group.calls) {
			if (internalNames.has(callee))
				incoming.get(callee).add(name);
		}
	}

	return { defs, groups, incoming, internalNames, functionPointerTables };
}

function renderTree(inputFiles, graph) {
	// Prepare stable source-order sorting for the tree.
	const { defs, groups, incoming, internalNames, functionPointerTables } = graph;
	const fileOrder = new Map(inputFiles.map((fileName, index) => [fileName, index]));
	const duplicateGroups = [...groups.values()].filter(group => group.defs.length > 1).map(group => group.name).sort();

	function formatNameSet(names) {
		// Sort and render a set of in-scope function names.
		const sorted = [...names].sort(byLocation);
		return sorted.length ? sorted.join(", ") : "(none)";
	}

	function formatTableEdges(group) {
		// Invert table-call edges so each table lists the functions it contributes.
		const byTable = new Map();
		for (const [callee, tableNames] of group.tableCalls) {
			for (const tableName of tableNames) {
				if (!byTable.has(tableName))
					byTable.set(tableName, new Set());
				byTable.get(tableName).add(callee);
			}
		}

		return [...byTable.entries()]
			.sort((a, b) => a[0].localeCompare(b[0]))
			.map(([tableName, callees]) => `${tableName}: ${formatNameSet(callees)}`);
	}

	function edgeNote(parentName, childName) {
		// Annotate tree edges that are not ordinary direct calls.
		const parent = groups.get(parentName);
		if (!parent || parent.directCalls.has(childName))
			return "";

		const notes = [];
		if (parent.macroCalls.has(childName))
			notes.push("macro");
		if (parent.referenceCalls.has(childName))
			notes.push("ref");
		if (parent.tableCalls.has(childName))
			notes.push(`table: ${[...parent.tableCalls.get(childName)].sort().join(", ")}`);

		return notes.length ? ` [${notes.join("; ")}]` : "";
	}

	// Sort functions by source file order and earliest line.
	function byLocation(a, b) {
		const fileA = firstDefinitionFile(groups, a);
		const fileB = firstDefinitionFile(groups, b);
		const orderA = fileOrder.has(fileA) ? fileOrder.get(fileA) : 99;
		const orderB = fileOrder.has(fileB) ? fileOrder.get(fileB) : 99;
		if (orderA !== orderB)
			return orderA - orderB;

		return firstDefinitionLine(groups, a) - firstDefinitionLine(groups, b) || a.localeCompare(b);
	}

	// Cache sorted callees and root functions.
	for (const group of groups.values())
		group.callList = [...group.calls].sort(byLocation);
	const roots = [...internalNames].filter(name => incoming.get(name).size === 0).sort(byLocation);
	const macroEdgeCount = [...groups.values()].reduce((sum, group) => sum + group.macroCalls.size, 0);
	const referenceEdgeCount = [...groups.values()].reduce((sum, group) => sum + group.referenceCalls.size, 0);
	const tableEdgeCount = [...groups.values()].reduce((sum, group) => sum + group.tableCalls.size, 0);

	// Start the report with scope and parser metadata.
	const lines = [];
	lines.push("Static function call tree");
	lines.push("");
	lines.push("Generated from:");
	for (const fileName of inputFiles)
		lines.push(`  - ${fileName}`);
	lines.push("");
	lines.push("Scope: direct and inferred dependencies between functions defined in the generated-source scope above.");
	lines.push("Function-like macros that expand to in-scope function calls are followed for root/callee purposes.");
	lines.push("Bare in-scope function references and parsed top-level function-pointer tables are followed as indirect edges.");
	lines.push("Calls through opaque function-pointer variables are still omitted when the target cannot be inferred.");
	lines.push("Roots are functions with no incoming in-scope dependency edge. Repeated nodes are not expanded again.");
	lines.push("");
	lines.push(`Function definitions parsed: ${defs.length}`);
	lines.push(`Logical functions: ${groups.size}`);
	lines.push(`Root functions: ${roots.length}`);
	lines.push(`Function pointer tables parsed: ${functionPointerTables.length}`);
	lines.push(`Indirect reference edges: ${referenceEdgeCount}`);
	lines.push(`Function pointer table edges: ${tableEdgeCount}`);
	lines.push(`Macro-expanded edges: ${macroEdgeCount}`);
	lines.push(`Duplicate-name groups: ${duplicateGroups.length ? duplicateGroups.join(", ") : "none"}`);
	lines.push("");
	lines.push("Call Tree");
	lines.push("========");

	const expanded = new Set();

	function emit(name, prefix, isLast, stack, depth, parentName) {
		// Render one function and recursively render unexpanded children.
		const connector = depth === 0 ? "" : (isLast ? "`-- " : "|-- ");
		const cycle = stack.includes(name);
		const repeated = expanded.has(name) && !cycle;
		const note = parentName ? edgeNote(parentName, name) : "";
		lines.push(`${prefix}${connector}${labelFor(groups, name)}${note}${cycle ? " [cycle]" : repeated ? " [already expanded]" : ""}`);
		if (cycle || repeated)
			return;

		expanded.add(name);
		const childPrefix = depth === 0 ? "" : prefix + (isLast ? "    " : "|   ");
		const children = groups.get(name).callList;
		for (let i = 0; i < children.length; ++i)
			emit(children[i], childPrefix, i === children.length - 1, [...stack, name], depth + 1, name);
	}

	// Emit each root as a separate top-level tree.
	for (let i = 0; i < roots.length; ++i) {
		if (i)
			lines.push("");
		emit(roots[i], "", true, [], 0, null);
	}

	// Emit any unreachable cycles or non-root islands after the root forest.
	const notReached = [...internalNames].filter(name => !expanded.has(name)).sort(byLocation);
	if (notReached.length) {
		lines.push("");
		lines.push("Unexpanded Cyclic Or Non-root Functions");
		lines.push("======================================");
		for (const name of notReached)
			emit(name, "", true, [], 0, null);
	}

	// Emit a flat union index that is easier to grep than the visual tree.
	lines.push("");
	lines.push("All Internal Dependencies");
	lines.push("=========================");
	for (const name of [...internalNames].sort(byLocation)) {
		const callees = groups.get(name).callList;
		lines.push(`${labelFor(groups, name)} -> ${callees.length ? callees.join(", ") : "(none)"}`);
	}

	// Emit the direct-call-only index for callers that need the old view.
	lines.push("");
	lines.push("Direct Internal Callees");
	lines.push("=======================");
	for (const name of [...internalNames].sort(byLocation)) {
		const group = groups.get(name);
		lines.push(`${labelFor(groups, name)} -> ${formatNameSet(group.directCalls)}`);
	}

	// Emit indirect edge details so non-call dependencies can be audited.
	lines.push("");
	lines.push("Indirect Internal Dependencies");
	lines.push("==============================");
	let indirectLineCount = 0;
	for (const name of [...internalNames].sort(byLocation)) {
		const group = groups.get(name);
		const parts = [];
		if (group.macroCalls.size)
			parts.push(`macro: ${formatNameSet(group.macroCalls)}`);
		if (group.referenceCalls.size)
			parts.push(`refs: ${formatNameSet(group.referenceCalls)}`);
		for (const tablePart of formatTableEdges(group))
			parts.push(`table ${tablePart}`);
		if (parts.length) {
			lines.push(`${labelFor(groups, name)} -> ${parts.join("; ")}`);
			++indirectLineCount;
		}
	}
	if (!indirectLineCount)
		lines.push("(none)");

	// Emit the function-pointer tables that contributed table edges.
	lines.push("");
	lines.push("Function Pointer Tables");
	lines.push("=======================");
	if (functionPointerTables.length) {
		const tables = functionPointerTables.slice().sort((a, b) => {
			const orderA = fileOrder.has(a.fileName) ? fileOrder.get(a.fileName) : 99;
			const orderB = fileOrder.has(b.fileName) ? fileOrder.get(b.fileName) : 99;
			return orderA - orderB || a.line - b.line || a.name.localeCompare(b.name);
		});
		for (const table of tables)
			lines.push(`${tableLabel(table)} -> ${formatNameSet(table.refs)}`);
	} else {
		lines.push("(none)");
	}

	return lines.join("\n") + "\n";
}

function main() {
	// Parse CLI arguments and print help when requested.
	const config = parseArgs(process.argv.slice(2));
	if (config.help) {
		printHelp();
		return;
	}

	// Expand wildcard inputs and optionally follow quoted local includes.
	const directInputPaths = expandInputPatterns(config.inputs);
	const includeResult = config.followIncludes ? expandIncludedFiles(directInputPaths) : { files: directInputPaths, missing: [] };
	const inputPaths = includeResult.files;
	const outputPath = path.resolve(config.output);
	const reportInputs = inputPaths.slice();

	// Generate the graph and write the report with LF line endings.
	const graph = buildGraph(inputPaths);
	const report = renderTree(reportInputs, graph);
	fs.mkdirSync(path.dirname(outputPath), { recursive: true });
	fs.writeFileSync(outputPath, report, "utf8");

	// Report the completed output path to the caller.
	console.log(`Wrote ${normalizePath(outputPath)}`);
	console.log(`Scanned ${inputPaths.length} files.`);
	console.log(`Parsed ${graph.defs.length} definitions into ${graph.groups.size} logical functions.`);
	if (includeResult.missing.length) {
		console.warn(`Skipped ${includeResult.missing.length} missing quoted includes:`);
		for (const includePath of includeResult.missing)
			console.warn(`  ${includePath}`);
	}
}

main();
