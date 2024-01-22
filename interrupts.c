#include "interrupts.h"
#include "stdio.h"
#include "io.h"

typedef struct {
  uint16 offset_low;
  uint16 selector;
  uint8 zero;
  uint8 type_attr;
  uint16 offset_high;
} __attribute__((packed)) idt_entry_t;

idt_entry_t idt[256];

void interrupt empty_exception_handler(interrupt_frame_t* frame) {
  printf("Interrupt!\n");
  printf("ip: %x\n", frame->ip);
  printf("cs: %x\n", frame->cs);
  printf("flags: %x\n", frame->flags);
  printf("sp: %x\n", frame->sp);
  printf("ss: %x\n", frame->ss);
}


typedef struct {
  uint16 limit;
  uint32 base;
} __attribute__((packed)) idt_ptr_t;

enum interrupt_gate_type {
  INTERRUPT_GATE_TYPE_TASK = 0x5,
  INTERRUPT_GATE_TYPE_INTERRUPT = 0xE,
  INTERRUPT_GATE_TYPE_TRAP = 0xF,
};

void interrupts_set_handler(uint8 vector, interrupt_handler handler, enum interrupt_gate_type type) {
  uint32 handler_int = (uint32)handler;
  idt[vector].offset_low = handler_int & 0xFFFF;
  idt[vector].selector = 0x08;
  idt[vector].zero = 0;
  idt[vector].type_attr = 0x80 | type;
  idt[vector].offset_high = (handler_int >> 16) & 0xFFFF;
}

void interrupts_set_interrupt_handler(uint8 vector, interrupt_handler handler) {
  interrupts_set_handler(vector, handler, INTERRUPT_GATE_TYPE_INTERRUPT);
}

void interrupts_set_trap_handler(uint8 vector, interrupt_handler handler) {
  interrupts_set_handler(vector, handler, INTERRUPT_GATE_TYPE_TRAP);
}

static void print_interrupt_frame(interrupt_frame_t* frame) {
  printf("ip: %x\n", frame->ip);
  printf("cs: %x\n", frame->cs);
  printf("flags: %x\n", frame->flags);
  printf("sp: %x\n", frame->sp);
  printf("ss: %x\n", frame->ss);
}

void halt(void) {
  asm volatile("cli");
  while (1) {
    asm volatile("hlt");
  }
}
const char INTERRUPT_MSG[] = "Interrupt %d\n";

#define DEFAULT_INTERRUPT(num) \
  void interrupt handle_interrupt_##num(interrupt_frame_t* frame) { \
    printf(INTERRUPT_MSG, num); \
    print_interrupt_frame(frame); \
    halt(); \
  }

DEFAULT_INTERRUPT(0)
DEFAULT_INTERRUPT(1)
DEFAULT_INTERRUPT(2)
DEFAULT_INTERRUPT(3)
DEFAULT_INTERRUPT(4)
DEFAULT_INTERRUPT(5)
DEFAULT_INTERRUPT(6)
DEFAULT_INTERRUPT(7)
DEFAULT_INTERRUPT(8)
DEFAULT_INTERRUPT(9)
DEFAULT_INTERRUPT(10)
DEFAULT_INTERRUPT(11)
DEFAULT_INTERRUPT(12)
DEFAULT_INTERRUPT(13)
DEFAULT_INTERRUPT(14)
DEFAULT_INTERRUPT(15)
DEFAULT_INTERRUPT(16)
DEFAULT_INTERRUPT(17)
DEFAULT_INTERRUPT(18)
DEFAULT_INTERRUPT(19)
DEFAULT_INTERRUPT(20)
DEFAULT_INTERRUPT(21)
DEFAULT_INTERRUPT(22)
DEFAULT_INTERRUPT(23)
DEFAULT_INTERRUPT(24)
DEFAULT_INTERRUPT(25)
DEFAULT_INTERRUPT(26)
DEFAULT_INTERRUPT(27)
DEFAULT_INTERRUPT(28)
DEFAULT_INTERRUPT(29)
DEFAULT_INTERRUPT(30)
DEFAULT_INTERRUPT(31)
DEFAULT_INTERRUPT(32)
DEFAULT_INTERRUPT(33)
DEFAULT_INTERRUPT(34)
DEFAULT_INTERRUPT(35)
DEFAULT_INTERRUPT(36)

DEFAULT_INTERRUPT(37)
DEFAULT_INTERRUPT(38)
void interrupt handle_interrupt_39(interrupt_frame_t* frame) {
  // printf("Interrupt 39, probably spurious\n");
  // Acknowledge the interrupt, this is probably spurious
  outb(0x20, 0x20);
}
DEFAULT_INTERRUPT(40)
DEFAULT_INTERRUPT(41)
DEFAULT_INTERRUPT(42)
DEFAULT_INTERRUPT(43)
DEFAULT_INTERRUPT(44)
DEFAULT_INTERRUPT(45)
DEFAULT_INTERRUPT(46)
DEFAULT_INTERRUPT(47)
DEFAULT_INTERRUPT(48)
DEFAULT_INTERRUPT(49)
DEFAULT_INTERRUPT(50)
DEFAULT_INTERRUPT(51)
DEFAULT_INTERRUPT(52)
DEFAULT_INTERRUPT(53)
DEFAULT_INTERRUPT(54)
DEFAULT_INTERRUPT(55)
DEFAULT_INTERRUPT(56)
DEFAULT_INTERRUPT(57)
DEFAULT_INTERRUPT(58)
DEFAULT_INTERRUPT(59)
DEFAULT_INTERRUPT(60)
DEFAULT_INTERRUPT(61)
DEFAULT_INTERRUPT(62)
DEFAULT_INTERRUPT(63)
DEFAULT_INTERRUPT(64)
DEFAULT_INTERRUPT(65)
DEFAULT_INTERRUPT(66)
DEFAULT_INTERRUPT(67)
DEFAULT_INTERRUPT(68)
DEFAULT_INTERRUPT(69)
DEFAULT_INTERRUPT(70)
DEFAULT_INTERRUPT(71)
DEFAULT_INTERRUPT(72)
DEFAULT_INTERRUPT(73)
DEFAULT_INTERRUPT(74)
DEFAULT_INTERRUPT(75)
DEFAULT_INTERRUPT(76)
DEFAULT_INTERRUPT(77)
DEFAULT_INTERRUPT(78)
DEFAULT_INTERRUPT(79)
DEFAULT_INTERRUPT(80)
DEFAULT_INTERRUPT(81)
DEFAULT_INTERRUPT(82)
DEFAULT_INTERRUPT(83)
DEFAULT_INTERRUPT(84)
DEFAULT_INTERRUPT(85)
DEFAULT_INTERRUPT(86)
DEFAULT_INTERRUPT(87)
DEFAULT_INTERRUPT(88)
DEFAULT_INTERRUPT(89)
DEFAULT_INTERRUPT(90)
DEFAULT_INTERRUPT(91)
DEFAULT_INTERRUPT(92)
DEFAULT_INTERRUPT(93)
DEFAULT_INTERRUPT(94)
DEFAULT_INTERRUPT(95)
DEFAULT_INTERRUPT(96)
DEFAULT_INTERRUPT(97)
DEFAULT_INTERRUPT(98)
DEFAULT_INTERRUPT(99)
DEFAULT_INTERRUPT(100)
DEFAULT_INTERRUPT(101)
DEFAULT_INTERRUPT(102)
DEFAULT_INTERRUPT(103)
DEFAULT_INTERRUPT(104)
DEFAULT_INTERRUPT(105)
DEFAULT_INTERRUPT(106)
DEFAULT_INTERRUPT(107)
DEFAULT_INTERRUPT(108)
DEFAULT_INTERRUPT(109)
DEFAULT_INTERRUPT(110)
DEFAULT_INTERRUPT(111)
DEFAULT_INTERRUPT(112)
DEFAULT_INTERRUPT(113)
DEFAULT_INTERRUPT(114)
DEFAULT_INTERRUPT(115)
DEFAULT_INTERRUPT(116)
DEFAULT_INTERRUPT(117)
DEFAULT_INTERRUPT(118)
DEFAULT_INTERRUPT(119)
DEFAULT_INTERRUPT(120)
DEFAULT_INTERRUPT(121)
DEFAULT_INTERRUPT(122)
DEFAULT_INTERRUPT(123)
DEFAULT_INTERRUPT(124)
DEFAULT_INTERRUPT(125)
DEFAULT_INTERRUPT(126)
DEFAULT_INTERRUPT(127)
DEFAULT_INTERRUPT(128)
DEFAULT_INTERRUPT(129)
DEFAULT_INTERRUPT(130)
DEFAULT_INTERRUPT(131)
DEFAULT_INTERRUPT(132)
DEFAULT_INTERRUPT(133)
DEFAULT_INTERRUPT(134)
DEFAULT_INTERRUPT(135)
DEFAULT_INTERRUPT(136)
DEFAULT_INTERRUPT(137)
DEFAULT_INTERRUPT(138)
DEFAULT_INTERRUPT(139)
DEFAULT_INTERRUPT(140)
DEFAULT_INTERRUPT(141)
DEFAULT_INTERRUPT(142)
DEFAULT_INTERRUPT(143)
DEFAULT_INTERRUPT(144)
DEFAULT_INTERRUPT(145)
DEFAULT_INTERRUPT(146)
DEFAULT_INTERRUPT(147)
DEFAULT_INTERRUPT(148)
DEFAULT_INTERRUPT(149)
DEFAULT_INTERRUPT(150)
DEFAULT_INTERRUPT(151)
DEFAULT_INTERRUPT(152)
DEFAULT_INTERRUPT(153)
DEFAULT_INTERRUPT(154)
DEFAULT_INTERRUPT(155)
DEFAULT_INTERRUPT(156)
DEFAULT_INTERRUPT(157)
DEFAULT_INTERRUPT(158)
DEFAULT_INTERRUPT(159)
DEFAULT_INTERRUPT(160)
DEFAULT_INTERRUPT(161)
DEFAULT_INTERRUPT(162)
DEFAULT_INTERRUPT(163)
DEFAULT_INTERRUPT(164)
DEFAULT_INTERRUPT(165)
DEFAULT_INTERRUPT(166)
DEFAULT_INTERRUPT(167)
DEFAULT_INTERRUPT(168)
DEFAULT_INTERRUPT(169)
DEFAULT_INTERRUPT(170)
DEFAULT_INTERRUPT(171)
DEFAULT_INTERRUPT(172)
DEFAULT_INTERRUPT(173)
DEFAULT_INTERRUPT(174)
DEFAULT_INTERRUPT(175)
DEFAULT_INTERRUPT(176)
DEFAULT_INTERRUPT(177)
DEFAULT_INTERRUPT(178)
DEFAULT_INTERRUPT(179)
DEFAULT_INTERRUPT(180)
DEFAULT_INTERRUPT(181)
DEFAULT_INTERRUPT(182)
DEFAULT_INTERRUPT(183)
DEFAULT_INTERRUPT(184)
DEFAULT_INTERRUPT(185)
DEFAULT_INTERRUPT(186)
DEFAULT_INTERRUPT(187)
DEFAULT_INTERRUPT(188)
DEFAULT_INTERRUPT(189)
DEFAULT_INTERRUPT(190)
DEFAULT_INTERRUPT(191)
DEFAULT_INTERRUPT(192)
DEFAULT_INTERRUPT(193)
DEFAULT_INTERRUPT(194)
DEFAULT_INTERRUPT(195)
DEFAULT_INTERRUPT(196)
DEFAULT_INTERRUPT(197)
DEFAULT_INTERRUPT(198)
DEFAULT_INTERRUPT(199)
DEFAULT_INTERRUPT(200)
DEFAULT_INTERRUPT(201)
DEFAULT_INTERRUPT(202)
DEFAULT_INTERRUPT(203)
DEFAULT_INTERRUPT(204)
DEFAULT_INTERRUPT(205)
DEFAULT_INTERRUPT(206)
DEFAULT_INTERRUPT(207)
DEFAULT_INTERRUPT(208)
DEFAULT_INTERRUPT(209)
DEFAULT_INTERRUPT(210)
DEFAULT_INTERRUPT(211)
DEFAULT_INTERRUPT(212)
DEFAULT_INTERRUPT(213)
DEFAULT_INTERRUPT(214)
DEFAULT_INTERRUPT(215)
DEFAULT_INTERRUPT(216)
DEFAULT_INTERRUPT(217)
DEFAULT_INTERRUPT(218)
DEFAULT_INTERRUPT(219)
DEFAULT_INTERRUPT(220)
DEFAULT_INTERRUPT(221)
DEFAULT_INTERRUPT(222)
DEFAULT_INTERRUPT(223)
DEFAULT_INTERRUPT(224)
DEFAULT_INTERRUPT(225)
DEFAULT_INTERRUPT(226)
DEFAULT_INTERRUPT(227)
DEFAULT_INTERRUPT(228)
DEFAULT_INTERRUPT(229)
DEFAULT_INTERRUPT(230)
DEFAULT_INTERRUPT(231)
DEFAULT_INTERRUPT(232)
DEFAULT_INTERRUPT(233)
DEFAULT_INTERRUPT(234)
DEFAULT_INTERRUPT(235)
DEFAULT_INTERRUPT(236)
DEFAULT_INTERRUPT(237)
DEFAULT_INTERRUPT(238)
DEFAULT_INTERRUPT(239)
DEFAULT_INTERRUPT(240)
DEFAULT_INTERRUPT(241)
DEFAULT_INTERRUPT(242)
DEFAULT_INTERRUPT(243)
DEFAULT_INTERRUPT(244)
DEFAULT_INTERRUPT(245)
DEFAULT_INTERRUPT(246)
DEFAULT_INTERRUPT(247)
DEFAULT_INTERRUPT(248)
DEFAULT_INTERRUPT(249)
DEFAULT_INTERRUPT(250)
DEFAULT_INTERRUPT(251)
DEFAULT_INTERRUPT(252)
DEFAULT_INTERRUPT(253)
DEFAULT_INTERRUPT(254)
DEFAULT_INTERRUPT(255)


const interrupt_handler default_interrupt_handlers[256] = {
  [0] = handle_interrupt_0,
  [1] = handle_interrupt_1,
  [2] = handle_interrupt_2,
  [3] = handle_interrupt_3,
  [4] = handle_interrupt_4,
  [5] = handle_interrupt_5,
  [6] = handle_interrupt_6,
  [7] = handle_interrupt_7,
  [8] = handle_interrupt_8,
  [9] = handle_interrupt_9,
  [10] = handle_interrupt_10,
  [11] = handle_interrupt_11,
  [12] = handle_interrupt_12,
  [13] = handle_interrupt_13,
  [14] = handle_interrupt_14,
  [15] = handle_interrupt_15,
  [16] = handle_interrupt_16,
  [17] = handle_interrupt_17,
  [18] = handle_interrupt_18,
  [19] = handle_interrupt_19,
  [20] = handle_interrupt_20,
  [21] = handle_interrupt_21,
  [22] = handle_interrupt_22,
  [23] = handle_interrupt_23,
  [24] = handle_interrupt_24,
  [25] = handle_interrupt_25,
  [26] = handle_interrupt_26,
  [27] = handle_interrupt_27,
  [28] = handle_interrupt_28,
  [29] = handle_interrupt_29,
  [30] = handle_interrupt_30,
  [31] = handle_interrupt_31,
  [32] = handle_interrupt_32,
  [33] = handle_interrupt_33,
  [34] = handle_interrupt_34,
  [35] = handle_interrupt_35,
  [36] = handle_interrupt_36,
  [37] = handle_interrupt_37,
  [38] = handle_interrupt_38,
  [39] = handle_interrupt_39,
  [40] = handle_interrupt_40,
  [41] = handle_interrupt_41,
  [42] = handle_interrupt_42,
  [43] = handle_interrupt_43,
  [44] = handle_interrupt_44,
  [45] = handle_interrupt_45,
  [46] = handle_interrupt_46,
  [47] = handle_interrupt_47,
  [48] = handle_interrupt_48,
  [49] = handle_interrupt_49,
  [50] = handle_interrupt_50,
  [51] = handle_interrupt_51,
  [52] = handle_interrupt_52,
  [53] = handle_interrupt_53,
  [54] = handle_interrupt_54,
  [55] = handle_interrupt_55,
  [56] = handle_interrupt_56,
  [57] = handle_interrupt_57,
  [58] = handle_interrupt_58,
  [59] = handle_interrupt_59,
  [60] = handle_interrupt_60,
  [61] = handle_interrupt_61,
  [62] = handle_interrupt_62,
  [63] = handle_interrupt_63,
  [64] = handle_interrupt_64,
  [65] = handle_interrupt_65,
  [66] = handle_interrupt_66,
  [67] = handle_interrupt_67,
  [68] = handle_interrupt_68,
  [69] = handle_interrupt_69,
  [70] = handle_interrupt_70,
  [71] = handle_interrupt_71,
  [72] = handle_interrupt_72,
  [73] = handle_interrupt_73,
  [74] = handle_interrupt_74,
  [75] = handle_interrupt_75,
  [76] = handle_interrupt_76,
  [77] = handle_interrupt_77,
  [78] = handle_interrupt_78,
  [79] = handle_interrupt_79,
  [80] = handle_interrupt_80,
  [81] = handle_interrupt_81,
  [82] = handle_interrupt_82,
  [83] = handle_interrupt_83,
  [84] = handle_interrupt_84,
  [85] = handle_interrupt_85,
  [86] = handle_interrupt_86,
  [87] = handle_interrupt_87,
  [88] = handle_interrupt_88,
  [89] = handle_interrupt_89,
  [90] = handle_interrupt_90,
  [91] = handle_interrupt_91,
  [92] = handle_interrupt_92,
  [93] = handle_interrupt_93,
  [94] = handle_interrupt_94,
  [95] = handle_interrupt_95,
  [96] = handle_interrupt_96,
  [97] = handle_interrupt_97,
  [98] = handle_interrupt_98,
  [99] = handle_interrupt_99,
  [100] = handle_interrupt_100,
  [101] = handle_interrupt_101,
  [102] = handle_interrupt_102,
  [103] = handle_interrupt_103,
  [104] = handle_interrupt_104,
  [105] = handle_interrupt_105,
  [106] = handle_interrupt_106,
  [107] = handle_interrupt_107,
  [108] = handle_interrupt_108,
  [109] = handle_interrupt_109,
  [110] = handle_interrupt_110,
  [111] = handle_interrupt_111,
  [112] = handle_interrupt_112,
  [113] = handle_interrupt_113,
  [114] = handle_interrupt_114,
  [115] = handle_interrupt_115,
  [116] = handle_interrupt_116,
  [117] = handle_interrupt_117,
  [118] = handle_interrupt_118,
  [119] = handle_interrupt_119,
  [120] = handle_interrupt_120,
  [121] = handle_interrupt_121,
  [122] = handle_interrupt_122,
  [123] = handle_interrupt_123,
  [124] = handle_interrupt_124,
  [125] = handle_interrupt_125,
  [126] = handle_interrupt_126,
  [127] = handle_interrupt_127,
  [128] = handle_interrupt_128,
  [129] = handle_interrupt_129,
  [130] = handle_interrupt_130,
  [131] = handle_interrupt_131,
  [132] = handle_interrupt_132,
  [133] = handle_interrupt_133,
  [134] = handle_interrupt_134,
  [135] = handle_interrupt_135,
  [136] = handle_interrupt_136,
  [137] = handle_interrupt_137,
  [138] = handle_interrupt_138,
  [139] = handle_interrupt_139,
  [140] = handle_interrupt_140,
  [141] = handle_interrupt_141,
  [142] = handle_interrupt_142,
  [143] = handle_interrupt_143,
  [144] = handle_interrupt_144,
  [145] = handle_interrupt_145,
  [146] = handle_interrupt_146,
  [147] = handle_interrupt_147,
  [148] = handle_interrupt_148,
  [149] = handle_interrupt_149,
  [150] = handle_interrupt_150,
  [151] = handle_interrupt_151,
  [152] = handle_interrupt_152,
  [153] = handle_interrupt_153,
  [154] = handle_interrupt_154,
  [155] = handle_interrupt_155,
  [156] = handle_interrupt_156,
  [157] = handle_interrupt_157,
  [158] = handle_interrupt_158,
  [159] = handle_interrupt_159,
  [160] = handle_interrupt_160,
  [161] = handle_interrupt_161,
  [162] = handle_interrupt_162,
  [163] = handle_interrupt_163,
  [164] = handle_interrupt_164,
  [165] = handle_interrupt_165,
  [166] = handle_interrupt_166,
  [167] = handle_interrupt_167,
  [168] = handle_interrupt_168,
  [169] = handle_interrupt_169,
  [170] = handle_interrupt_170,
  [171] = handle_interrupt_171,
  [172] = handle_interrupt_172,
  [173] = handle_interrupt_173,
  [174] = handle_interrupt_174,
  [175] = handle_interrupt_175,
  [176] = handle_interrupt_176,
  [177] = handle_interrupt_177,
  [178] = handle_interrupt_178,
  [179] = handle_interrupt_179,
  [180] = handle_interrupt_180,
  [181] = handle_interrupt_181,
  [182] = handle_interrupt_182,
  [183] = handle_interrupt_183,
  [184] = handle_interrupt_184,
  [185] = handle_interrupt_185,
  [186] = handle_interrupt_186,
  [187] = handle_interrupt_187,
  [188] = handle_interrupt_188,
  [189] = handle_interrupt_189,
  [190] = handle_interrupt_190,
  [191] = handle_interrupt_191,
  [192] = handle_interrupt_192,
  [193] = handle_interrupt_193,
  [194] = handle_interrupt_194,
  [195] = handle_interrupt_195,
  [196] = handle_interrupt_196,
  [197] = handle_interrupt_197,
  [198] = handle_interrupt_198,
  [199] = handle_interrupt_199,
  [200] = handle_interrupt_200,
  [201] = handle_interrupt_201,
  [202] = handle_interrupt_202,
  [203] = handle_interrupt_203,
  [204] = handle_interrupt_204,
  [205] = handle_interrupt_205,
  [206] = handle_interrupt_206,
  [207] = handle_interrupt_207,
  [208] = handle_interrupt_208,
  [209] = handle_interrupt_209,
  [210] = handle_interrupt_210,
  [211] = handle_interrupt_211,
  [212] = handle_interrupt_212,
  [213] = handle_interrupt_213,
  [214] = handle_interrupt_214,
  [215] = handle_interrupt_215,
  [216] = handle_interrupt_216,
  [217] = handle_interrupt_217,
  [218] = handle_interrupt_218,
  [219] = handle_interrupt_219,
  [220] = handle_interrupt_220,
  [221] = handle_interrupt_221,
  [222] = handle_interrupt_222,
  [223] = handle_interrupt_223,
  [224] = handle_interrupt_224,
  [225] = handle_interrupt_225,
  [226] = handle_interrupt_226,
  [227] = handle_interrupt_227,
  [228] = handle_interrupt_228,
  [229] = handle_interrupt_229,
  [230] = handle_interrupt_230,
  [231] = handle_interrupt_231,
  [232] = handle_interrupt_232,
  [233] = handle_interrupt_233,
  [234] = handle_interrupt_234,
  [235] = handle_interrupt_235,
  [236] = handle_interrupt_236,
  [237] = handle_interrupt_237,
  [238] = handle_interrupt_238,
  [239] = handle_interrupt_239,
  [240] = handle_interrupt_240,
  [241] = handle_interrupt_241,
  [242] = handle_interrupt_242,
  [243] = handle_interrupt_243,
  [244] = handle_interrupt_244,
  [245] = handle_interrupt_245,
  [246] = handle_interrupt_246,
  [247] = handle_interrupt_247,
  [248] = handle_interrupt_248,
  [249] = handle_interrupt_249,
  [250] = handle_interrupt_250,
  [251] = handle_interrupt_251,
  [252] = handle_interrupt_252,
  [253] = handle_interrupt_253,
  [254] = handle_interrupt_254,
  [255] = handle_interrupt_255,
};

void interrupt handle_divide_by_zero(interrupt_frame_t* frame) {
  printf("Divide by zero!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_debug(interrupt_frame_t* frame) {
  printf("Debug!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_nmi(interrupt_frame_t* frame) {
  printf("NMI!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_breakpoint(interrupt_frame_t* frame) {
  printf("Breakpoint!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_overflow(interrupt_frame_t* frame) {
  printf("Overflow!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_bound_range_exceeded(interrupt_frame_t* frame) {
  printf("Bound range exceeded!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_invalid_opcode(interrupt_frame_t* frame) {
  printf("Invalid opcode!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_device_not_available(interrupt_frame_t* frame) {
  printf("Device not available!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_double_fault(interrupt_frame_t* frame) {
  printf("Double fault!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_coprocessor_segment_overrun(interrupt_frame_t* frame) {
  printf("Coprocessor segment overrun!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_invalid_tss(interrupt_frame_t* frame) {
  printf("Invalid TSS!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_segment_not_present(interrupt_frame_t* frame) {
  printf("Segment not present!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_stack_segment_fault(interrupt_frame_t* frame) {
  printf("Stack segment fault!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_general_protection_fault(interrupt_frame_t* frame) {
  printf("General protection fault!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_page_fault(interrupt_frame_t* frame, uint32 error_code) {
  printf("Page fault!\n");
  print_interrupt_frame(frame);
  printf("error code: %x\n", error_code);
  halt();
}

void interrupt handle_x87_floating_point_exception(interrupt_frame_t* frame) {
  printf("x87 floating point exception!\n");
  print_interrupt_frame(frame);
  halt();
}

void interrupt handle_alignment_check(interrupt_frame_t* frame, uint32 error_code) {
  printf("Alignment check!\n");
  print_interrupt_frame(frame);
  printf("error code: %x\n", error_code);
  halt();
}

void interrupt handle_machine_check(interrupt_frame_t* frame) {
  printf("Machine check!\n");
  print_interrupt_frame(frame);
}

void interrupt handle_simd_floating_point_exception(interrupt_frame_t* frame) {
  printf("SIMD floating point exception!\n");
  print_interrupt_frame(frame);
}

void interrupt handle_virtualization_exception(interrupt_frame_t* frame) {
  printf("Virtualization exception!\n");
  print_interrupt_frame(frame);
}

void interrupt handle_security_exception(interrupt_frame_t* frame) {
  printf("Security exception!\n");
  print_interrupt_frame(frame);
}

static void __attribute((no_caller_saved_registers)) check_timer_callbacks(void);

int watchdog_counter = 0;
int watchdog_enabled = 0;
unsigned int timer_ticks = 0;
void interrupt handle_timer(interrupt_frame_t* frame) {
  if (watchdog_enabled) {
    watchdog_counter++;
    if (watchdog_counter > 1000) {
      printf("Watchdog!\n");
      print_interrupt_frame(frame);
      halt();
    }
  }
  timer_ticks++;
  check_timer_callbacks();
  // Acknowledge the interrupt
  outb(0x20, 0x20);
}

void interrupts_enable_watchdog(void) {
  watchdog_enabled = 1;
}

void interrupts_disable_watchdog(void) {
  watchdog_enabled = 0;
}

void interrupts_watchdog_reset(void) {
  watchdog_counter = 0;
}

void interrupts_init(void) {
  // Set up the IDT pointer:
  idt_ptr_t idt_ptr;
  idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
  idt_ptr.base = (uint32)&idt;

  for (int i = 0; i < 256; i++) {
    interrupts_set_interrupt_handler(i, default_interrupt_handlers[i]);
  }
  interrupts_set_trap_handler(0, handle_divide_by_zero);
  interrupts_set_trap_handler(1, handle_debug);
  interrupts_set_interrupt_handler(2, handle_nmi);
  interrupts_set_trap_handler(3, handle_breakpoint);
  interrupts_set_trap_handler(4, handle_overflow);
  interrupts_set_trap_handler(5, handle_bound_range_exceeded);
  interrupts_set_trap_handler(6, handle_invalid_opcode);
  interrupts_set_trap_handler(7, handle_device_not_available);
  interrupts_set_interrupt_handler(8, handle_double_fault);
  interrupts_set_trap_handler(9, handle_coprocessor_segment_overrun);
  interrupts_set_trap_handler(10, handle_invalid_tss);
  interrupts_set_trap_handler(11, handle_segment_not_present);
  interrupts_set_trap_handler(12, handle_stack_segment_fault);
  interrupts_set_trap_handler(13, handle_general_protection_fault);
  interrupts_set_trap_handler(14, (void*)handle_page_fault);
  interrupts_set_trap_handler(16, handle_x87_floating_point_exception);
  interrupts_set_trap_handler(17, (void*)handle_alignment_check);
  interrupts_set_interrupt_handler(18, (void*)handle_machine_check);
  interrupts_set_trap_handler(19, handle_simd_floating_point_exception);
  interrupts_set_trap_handler(20, handle_virtualization_exception);
  interrupts_set_trap_handler(30, handle_security_exception);
  // Setup the PIT timer to interrupt every 1ms
  outb(0x43, 0x36);
  int divisor = 1193180 / 1000;
  outb(0x40, divisor & 0xFF);
  outb(0x40, (divisor >> 8) & 0xFF);

  interrupts_set_interrupt_handler(32, handle_timer);
  // Points the processor's internal register to the new IDT
  // Initialize the PIC
  outb(0x20, 0x11); //begin initialization
  outb(0xA0, 0x11);
  outb(0x21, 0x20); // master offset
  outb(0xA1, 0x28); // slave offset
  outb(0x21, 0x04); // master/slave wiring
  outb(0xA1, 0x02);
  outb(0x21, 0x01); //8088
  outb(0xA1, 0x01);
  outb(0x21, 0xFF); // mask all until enabled
  outb(0xA1, 0xFF);
  asm volatile("lidt %0" : : "m"(idt_ptr));
  printf("Interrupts initialized\n");
}

void interrupts_clear_handler(uint8 vector) {
  interrupts_set_interrupt_handler(vector, default_interrupt_handlers[vector]);
}

void interrupts_enable(void) {
  printf("Enabling interrupts\n");
  outb(0x21, 0x00);
  outb(0xA1, 0x00);
  asm volatile("sti");
}

void interrupts_disable(void) {
  printf("Disabling interrupts\n");
  outb(0x21, 0xFF);
  outb(0xA1, 0xFF);
  asm volatile("cli");
}

unsigned int interrupts_timer_ticks(void) {
  return timer_ticks;
}

#define MAX_TIMERS 8
timer_callback timer_callbacks[MAX_TIMERS] = {};
unsigned int timer_callback_intervals[MAX_TIMERS] = {};
unsigned int timer_callback_counters[MAX_TIMERS] = {};

int interrupts_register_timer_callback(timer_callback callback, unsigned int interval) {
  printf("Registering timer callback\n");
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (timer_callbacks[i] == NULL) {
      timer_callbacks[i] = callback;
      timer_callback_intervals[i] = interval;
      timer_callback_counters[i] = 0;
      printf("Registered timer callback %d\n", i);
      return i;
    }
  }
  return -1;
}

static void __attribute__((no_caller_saved_registers)) check_timer_callbacks(void) {
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (timer_callbacks[i] != NULL) {
      timer_callback_counters[i]++;
      if (timer_callback_counters[i] >= timer_callback_intervals[i]) {
        unsigned int new_interval = timer_callbacks[i](timer_callback_intervals[i]);
        if (new_interval == 0) {
          timer_callbacks[i] = NULL;
          continue;
        }
        timer_callback_intervals[i] = new_interval;
        timer_callback_counters[i] = 0;
      }
    }
  }
}

int interrupts_unregister_timer_callback(int id) {
  if (id < 0 || id >= 256) {
    return -1;
  }
  timer_callbacks[id] = NULL;
  return 0;
}

bool interrupts_enabled() {
  uint32 flags;
  // Query the interrupt flag
  asm volatile("pushf\n\t"
               "pop %0"
               : "=g"(flags));
  return flags & (1 << 9);
}
