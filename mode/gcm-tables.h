/*
 * Precomputed Multiplication Tables for the GHASH
 * primitive of the GCM mode of operation.
 */

#if WORDS_BIGENDIAN
#define W(left,right) (0x##left##right)
#else
#define W(left,right) (0x##right##left)
#endif

#  if GCM_TABLE_BITS == 8

static const uint16_t shift_table[0400] = {
	W(00,00),W(01,C2),W(03,84),W(02,46),W(07,08),W(06,CA),W(04,8C),W(05,4E),
	W(0E,10),W(0F,D2),W(0D,94),W(0C,56),W(09,18),W(08,DA),W(0A,9C),W(0B,5E),
	W(1C,20),W(1D,E2),W(1F,A4),W(1E,66),W(1B,28),W(1A,EA),W(18,AC),W(19,6E),
	W(12,30),W(13,F2),W(11,B4),W(10,76),W(15,38),W(14,FA),W(16,BC),W(17,7E),
	W(38,40),W(39,82),W(3B,C4),W(3A,06),W(3F,48),W(3E,8A),W(3C,CC),W(3D,0E),
	W(36,50),W(37,92),W(35,D4),W(34,16),W(31,58),W(30,9A),W(32,DC),W(33,1E),
	W(24,60),W(25,A2),W(27,E4),W(26,26),W(23,68),W(22,AA),W(20,EC),W(21,2E),
	W(2A,70),W(2B,B2),W(29,F4),W(28,36),W(2D,78),W(2C,BA),W(2E,FC),W(2F,3E),
	W(70,80),W(71,42),W(73,04),W(72,C6),W(77,88),W(76,4A),W(74,0C),W(75,CE),
	W(7E,90),W(7F,52),W(7D,14),W(7C,D6),W(79,98),W(78,5A),W(7A,1C),W(7B,DE),
	W(6C,A0),W(6D,62),W(6F,24),W(6E,E6),W(6B,A8),W(6A,6A),W(68,2C),W(69,EE),
	W(62,B0),W(63,72),W(61,34),W(60,F6),W(65,B8),W(64,7A),W(66,3C),W(67,FE),
	W(48,C0),W(49,02),W(4B,44),W(4A,86),W(4F,C8),W(4E,0A),W(4C,4C),W(4D,8E),
	W(46,D0),W(47,12),W(45,54),W(44,96),W(41,D8),W(40,1A),W(42,5C),W(43,9E),
	W(54,E0),W(55,22),W(57,64),W(56,A6),W(53,E8),W(52,2A),W(50,6C),W(51,AE),
	W(5A,F0),W(5B,32),W(59,74),W(58,B6),W(5D,F8),W(5C,3A),W(5E,7C),W(5F,BE),
	W(E1,00),W(E0,C2),W(E2,84),W(E3,46),W(E6,08),W(E7,CA),W(E5,8C),W(E4,4E),
	W(EF,10),W(EE,D2),W(EC,94),W(ED,56),W(E8,18),W(E9,DA),W(EB,9C),W(EA,5E),
	W(FD,20),W(FC,E2),W(FE,A4),W(FF,66),W(FA,28),W(FB,EA),W(F9,AC),W(F8,6E),
	W(F3,30),W(F2,F2),W(F0,B4),W(F1,76),W(F4,38),W(F5,FA),W(F7,BC),W(F6,7E),
	W(D9,40),W(D8,82),W(DA,C4),W(DB,06),W(DE,48),W(DF,8A),W(DD,CC),W(DC,0E),
	W(D7,50),W(D6,92),W(D4,D4),W(D5,16),W(D0,58),W(D1,9A),W(D3,DC),W(D2,1E),
	W(C5,60),W(C4,A2),W(C6,E4),W(C7,26),W(C2,68),W(C3,AA),W(C1,EC),W(C0,2E),
	W(CB,70),W(CA,B2),W(C8,F4),W(C9,36),W(CC,78),W(CD,BA),W(CF,FC),W(CE,3E),
	W(91,80),W(90,42),W(92,04),W(93,C6),W(96,88),W(97,4A),W(95,0C),W(94,CE),
	W(9F,90),W(9E,52),W(9C,14),W(9D,D6),W(98,98),W(99,5A),W(9B,1C),W(9A,DE),
	W(8D,A0),W(8C,62),W(8E,24),W(8F,E6),W(8A,A8),W(8B,6A),W(89,2C),W(88,EE),
	W(83,B0),W(82,72),W(80,34),W(81,F6),W(84,B8),W(85,7A),W(87,3C),W(86,FE),
	W(A9,C0),W(A8,02),W(AA,44),W(AB,86),W(AE,C8),W(AF,0A),W(AD,4C),W(AC,8E),
	W(A7,D0),W(A6,12),W(A4,54),W(A5,96),W(A0,D8),W(A1,1A),W(A3,5C),W(A2,9E),
	W(B5,E0),W(B4,22),W(B6,64),W(B7,A6),W(B2,E8),W(B3,2A),W(B1,6C),W(B0,AE),
	W(BB,F0),W(BA,32),W(B8,74),W(B9,B6),W(BC,F8),W(BD,3A),W(BF,7C),W(BE,BE)
};

#elif GCM_TABLE_BITS == 4

static const uint16_t shift_table[0020] = {
	W(00,00),W(1C,20),W(38,40),W(24,60),W(70,80),W(6C,A0),W(48,C0),W(54,E0),
	W(E1,00),W(FD,20),W(D9,40),W(C5,60),W(91,80),W(8D,A0),W(A9,C0),W(B5,E0)
};

#elif GCM_TABLE_BITS == 2

static const uint16_t shift_table[0004] = {
	W(00,00),W(70,80),W(E1,00),W(91,80)
};

#endif

#undef W


