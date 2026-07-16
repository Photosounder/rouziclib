param(
	[Parameter(Mandatory=$true)][string]$InputFile,
	[Parameter(Mandatory=$true)][string]$OutputFile
)

$bytes = [IO.File]::ReadAllBytes($InputFile)
if (($bytes.Length % 4) -ne 0) {
	throw "SPIR-V byte count must be divisible by four"
}

$words = [uint32[]]::new($bytes.Length / 4)
[Buffer]::BlockCopy($bytes, 0, $words, 0, $bytes.Length)
$builder = [Text.StringBuilder]::new($words.Length * 13)
[void]$builder.AppendLine('static const uint32_t vk_drawqueue_comp_spv[] = {')

for ($index=0; $index -lt $words.Length; $index++) {
	if (($index % 8) -eq 0) {
		[void]$builder.Append("`t")
	}
	[void]$builder.Append(('0x{0:x8}u' -f $words[$index]))
	if ($index -lt $words.Length-1) {
		[void]$builder.Append(', ')
	}
	if (($index % 8) -eq 7) {
		[void]$builder.AppendLine()
	}
}

if ((($words.Length-1) % 8) -ne 7) {
	[void]$builder.AppendLine()
}
[void]$builder.AppendLine('};')
[void]$builder.AppendLine('static const size_t vk_drawqueue_comp_spv_size = sizeof(vk_drawqueue_comp_spv);')
$output = $builder.ToString().Replace([Environment]::NewLine, "`n")
[IO.File]::WriteAllText($OutputFile, $output, [Text.UTF8Encoding]::new($false))
