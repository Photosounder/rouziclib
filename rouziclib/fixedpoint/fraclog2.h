const uint32_t lutsp = 10, prec = 23, abdp = 8;
static int32_t fraclog2[] = {-83886080, -83886080, -75497472, -70590451, -67108864, -64408335, -62201843, -60336280, -58720256, -57294822, -56019727, -54866264, -53813235, -52844542, -51947672, -51112706, -50331648, -49597957, -48906214, -48251881, -47631119, -47040651, -46477656, -45939692, -45424627, -44930591, -44455934, -43999193, -43559064, -43134382, -42724098, -42327269, -41943040, -41570635, -41209349, -40858535, -40517606, -40186018, -39863273, -39548913, -39242511, -38943676, -38652043, -38367272, -38089048, -37817077, -37551084, -37290812, -37036019, -36786480, -36541983, -36302327, -36067326, -35836801, -35610585, -35388520, -35170456, -34956252, -34745774, -34538893, -34335490, -34135449, -33938661, -33745022, -33554432, -33366797, -33182027, -33000036, -32820741, -32644063, -32469927, -32298262, -32128998, -31962068, -31797410, -31634962, -31474665, -31316464, -31160305, -31006134, -30853903, -30703564, -30555068, -30408373, -30263435, -30120212, -29978664, -29838753, -29700440, -29563691, -29428469, -29294742, -29162476, -29031640, -28902204, -28774137, -28647411, -28521998, -28397872, -28275006, -28153375, -28032954, -27913719, -27795648, -27678718, -27562906, -27448193, -27334556, -27221977, -27110435, -26999912, -26890389, -26781848, -26674272, -26567644, -26461947, -26357166, -26253284, -26150285, -26048157, -25946882, -25846448, -25746841, -25648047, -25550053, -25452846, -25356414, -25260744, -25165824, -25071643, -24978189, -24885452, -24793419, -24702081, -24611428, -24521448, -24432133, -24343471, -24255455, -24168074, -24081319, -23995182, -23909654, -23824726, -23740390, -23656637, -23573460, -23490851, -23408802, -23327305, -23246354, -23165940, -23086057, -23006698, -22927856, -22849525, -22771697, -22694366, -22617526, -22541172, -22465295, -22389892, -22314956, -22240480, -22166460, -22092891, -22019765, -21947079, -21874827, -21803004, -21731604, -21660623, -21590056, -21519898, -21450145, -21380791, -21311832, -21243264, -21175083, -21107283, -21039861, -20972813, -20906134, -20839820, -20773868, -20708273, -20643032, -20578141, -20513596, -20449393, -20385529, -20322000, -20258803, -20195934, -20133390, -20071168, -20009264, -19947675, -19886398, -19825430, -19764767, -19704407, -19644346, -19584582, -19525111, -19465932, -19407040, -19348434, -19290110, -19232066, -19174298, -19116806, -19059585, -19002633, -18945948, -18889527, -18833369, -18777469, -18721827, -18666439, -18611304, -18556418, -18501781, -18447389, -18393240, -18339333, -18285664, -18232233, -18179036, -18126072, -18073339, -18020835, -17968558, -17916505, -17864676, -17813067, -17761677, -17710505, -17659549, -17608805, -17558274, -17507953, -17457840, -17407934, -17358233, -17308735, -17259439, -17210343, -17161445, -17112744, -17064238, -17015926, -16967806, -16919876, -16872136, -16824583, -16777216, -16730034, -16683035, -16636218, -16589581, -16543124, -16496844, -16450740, -16404811, -16359056, -16313473, -16268062, -16222820, -16177746, -16132840, -16088100, -16043525, -15999113, -15954863, -15910775, -15866847, -15823078, -15779466, -15736011, -15692711, -15649566, -15606574, -15563735, -15521046, -15478508, -15436118, -15393876, -15351782, -15309833, -15268029, -15226369, -15184852, -15143477, -15102243, -15061149, -15020194, -14979377, -14938697, -14898154, -14857746, -14817472, -14777332, -14737325, -14697449, -14657705, -14618090, -14578605, -14539248, -14500019, -14460917, -14421940, -14383089, -14344362, -14305758, -14267277, -14228918, -14190681, -14152564, -14114566, -14076687, -14038927, -14001284, -13963758, -13926348, -13889053, -13851872, -13814806, -13777852, -13741012, -13704283, -13667665, -13631157, -13594760, -13558471, -13522291, -13486219, -13450254, -13414396, -13378643, -13342996, -13307453, -13272015, -13236680, -13201448, -13166318, -13131290, -13096363, -13061537, -13026810, -12992183, -12957654, -12923224, -12888892, -12854656, -12820518, -12786475, -12752527, -12718675, -12684917, -12651253, -12617683, -12584205, -12550819, -12517526, -12484324, -12451212, -12418191, -12385260, -12352418, -12319665, -12287001, -12254424, -12221935, -12189533, -12157217, -12124988, -12092844, -12060785, -12028811, -11996921, -11965115, -11933392, -11901752, -11870195, -11838720, -11807326, -11776014, -11744782, -11713631, -11682560, -11651568, -11620656, -11589822, -11559067, -11528390, -11497790, -11467267, -11436822, -11406452, -11376159, -11345941, -11315799, -11285731, -11255738, -11225819, -11195974, -11166202, -11136503, -11106877, -11077324, -11047842, -11018432, -10989093, -10959826, -10930629, -10901502, -10872445, -10843458, -10814539, -10785690, -10756910, -10728198, -10699553, -10670977, -10642467, -10614025, -10585649, -10557340, -10529097, -10500919, -10472807, -10444761, -10416779, -10388861, -10361008, -10333219, -10305493, -10277831, -10250232, -10222696, -10195222, -10167810, -10140461, -10113173, -10085946, -10058781, -10031676, -10004632, -9977648, -9950725, -9923861, -9897056, -9870311, -9843625, -9816997, -9790428, -9763917, -9737464, -9711069, -9684731, -9658451, -9632227, -9606060, -9579950, -9553895, -9527897, -9501955, -9476068, -9450236, -9424459, -9398737, -9373069, -9347456, -9321897, -9296392, -9270941, -9245542, -9220197, -9194906, -9169666, -9144480, -9119345, -9094263, -9069232, -9044254, -9019326, -8994450, -8969625, -8944851, -8920127, -8895454, -8870831, -8846258, -8821735, -8797261, -8772837, -8748462, -8724136, -8699859, -8675630, -8651450, -8627318, -8603234, -8579198, -8555209, -8531268, -8507374, -8483528, -8459728, -8435975, -8412268, -8388608, -8364994, -8341426, -8317904, -8294427, -8270996, -8247610, -8224269, -8200973, -8177722, -8154516, -8131353, -8108236, -8085162, -8062132, -8039146, -8016203, -7993304, -7970448, -7947635, -7924865, -7902138, -7879454, -7856812, -7834212, -7811654, -7789138, -7766664, -7744232, -7721841, -7699492, -7677184, -7654917, -7632690, -7610505, -7588360, -7566255, -7544191, -7522167, -7500183, -7478239, -7456334, -7434470, -7412644, -7390858, -7369111, -7347403, -7325734, -7304103, -7282512, -7260958, -7239443, -7217966, -7196528, -7175127, -7153764, -7132438, -7111150, -7089900, -7068686, -7047510, -7026371, -7005268, -6984203, -6963174, -6942181, -6921225, -6900305, -6879421, -6858573, -6837761, -6816985, -6796244, -6775539, -6754869, -6734234, -6713635, -6693070, -6672541, -6652046, -6631586, -6611160, -6590769, -6570412, -6550089, -6529801, -6509546, -6489325, -6469138, -6448984, -6428864, -6408777, -6388724, -6368704, -6348717, -6328763, -6308841, -6288953, -6269097, -6249273, -6229482, -6209724, -6189997, -6170303, -6150640, -6131010, -6111411, -6091844, -6072309, -6052805, -6033332, -6013891, -5994481, -5975102, -5955754, -5936436, -5917150, -5897894, -5878669, -5859475, -5840310, -5821176, -5802073, -5782999, -5763956, -5744942, -5725958, -5707004, -5688079, -5669184, -5650319, -5631483, -5612676, -5593898, -5575150, -5556430, -5537740, -5519078, -5500445, -5481840, -5463264, -5444717, -5426198, -5407707, -5389244, -5370810, -5352404, -5334025, -5315675, -5297352, -5279057, -5260789, -5242549, -5224337, -5206152, -5187994, -5169863, -5151760, -5133683, -5115634, -5097611, -5079615, -5061646, -5043703, -5025788, -5007898, -4990035, -4972198, -4954388, -4936604, -4918845, -4901113, -4883407, -4865727, -4848072, -4830443, -4812840, -4795262, -4777710, -4760184, -4742682, -4725206, -4707755, -4690329, -4672929, -4655553, -4638202, -4620876, -4603575, -4586298, -4569046, -4551819, -4534616, -4517438, -4500284, -4483154, -4466048, -4448967, -4431910, -4414876, -4397867, -4380881, -4363919, -4346981, -4330067, -4313176, -4296309, -4279465, -4262645, -4245848, -4229075, -4212324, -4195597, -4178893, -4162211, -4145553, -4128918, -4112305, -4095716, -4079149, -4062604, -4046082, -4029583, -4013106, -3996652, -3980220, -3963810, -3947423, -3931057, -3914714, -3898393, -3882093, -3865816, -3849561, -3833327, -3817115, -3800925, -3784756, -3768609, -3752484, -3736380, -3720297, -3704236, -3688196, -3672177, -3656179, -3640203, -3624247, -3608313, -3592399, -3576507, -3560635, -3544784, -3528954, -3513144, -3497355, -3481587, -3465839, -3450112, -3434405, -3418718, -3403052, -3387406, -3371780, -3356174, -3340589, -3325023, -3309478, -3293952, -3278446, -3262960, -3247494, -3232048, -3216621, -3201214, -3185827, -3170459, -3155111, -3139782, -3124472, -3109182, -3093911, -3078659, -3063427, -3048214, -3033019, -3017844, -3002688, -2987551, -2972433, -2957333, -2942252, -2927191, -2912147, -2897123, -2882117, -2867130, -2852161, -2837211, -2822279, -2807366, -2792471, -2777594, -2762736, -2747895, -2733073, -2718269, -2703484, -2688716, -2673966, -2659234, -2644520, -2629824, -2615146, -2600485, -2585843, -2571218, -2556610, -2542021, -2527448, -2512894, -2498357, -2483837, -2469335, -2454850, -2440382, -2425931, -2411498, -2397082, -2382684, -2368302, -2353937, -2339590, -2325259, -2310945, -2296649, -2282369, -2268106, -2253859, -2239630, -2225417, -2211221, -2197041, -2182878, -2168732, -2154602, -2140489, -2126392, -2112311, -2098247, -2084199, -2070168, -2056153, -2042154, -2028171, -2014204, -2000253, -1986319, -1972400, -1958497, -1944611, -1930740, -1916885, -1903046, -1889223, -1875416, -1861624, -1847848, -1834088, -1820343, -1806614, -1792900, -1779202, -1765520, -1751853, -1738201, -1724565, -1710944, -1697338, -1683748, -1670173, -1656613, -1643068, -1629539, -1616024, -1602525, -1589040, -1575571, -1562117, -1548677, -1535253, -1521843, -1508448, -1495068, -1481703, -1468353, -1455017, -1441696, -1428389, -1415097, -1401820, -1388557, -1375309, -1362076, -1348856, -1335652, -1322461, -1309285, -1296123, -1282976, -1269843, -1256724, -1243619, -1230529, -1217452, -1204390, -1191342, -1178308, -1165287, -1152281, -1139289, -1126311, -1113347, -1100396, -1087460, -1074537, -1061628, -1048732, -1035851, -1022983, -1010129, -997288, -984461, -971648, -958848, -946062, -933289, -920530, -907784, -895052, -882333, -869627, -856934, -844255, -831589, -818937, -806298, -793671, -781058, -768458, -755872, -743298, -730737, -718190, -705655, -693133, -680624, -668129, -655646, -643176, -630718, -618274, -605842, -593423, -581017, -568624, -556243, -543875, -531519, -519177, -506846, -494529, -482223, -469931, -457650, -445382, -433127, -420884, -408653, -396435, -384229, -372036, -359854, -347685, -335528, -323383, -311251, -299130, -287022, -274926, -262842, -250770, -238710, -226662, -214626, -202602, -190590, -178590, -166601, -154625, -142660, -130707, -118766, -106837, -94920, -83014, -71120, -59238, -47367, -35508, -23660, -11824, 0};