#include "interrupt.h"
#include "ata.h"
#include "io.h"
#include "output.h"

typedef struct {
  u16 offset_low;
  u16 selector;
  u8 zero;
  u8 type_attr;
  u16 offset_high;
} packed idt_entry;

idt_entry idt[256];
itr_handler itr_handlers[256] = {};
void **itr_handler_data[256] = {};

void itr_set_handler(u8 vector, itr_handler handler, void* data) {
  // printf("Setting handler for interrupt %d to %x\n", vector, handler);
  itr_handlers[vector] = handler;
  itr_handler_data[vector] = data;
}


itr_handler_irq irq_handlers[16] = {};
void **irq_handler_data[16] = {};


void itr_handle_irq(u8 unused vector, void* unused _frame, void* data) {
  u32 irq = (u32)data;
  if (irq_handlers[irq]) {
    irq_handlers[irq](irq, irq_handler_data[irq]);
  }
  if (irq >= 8) {
    outb(0xA0, 0x20);
  }
  outb(0x20, 0x20);
}

void itr_set_irq_handler(enum itr_irq irq, itr_handler_irq handler, void* data) {
  printf("Setting handler for irq %d to %x\n", irq, handler);
  irq_handlers[irq] = handler;
  irq_handler_data[irq] = data;
  itr_set_handler(irq + 0x20, (itr_handler) itr_handle_irq, (void*)irq);
  if (irq >= 8) {
    itr_set_real_mode_handler(irq + 0x70, (itr_handler_real_mode)itr_handle_irq, (void*)irq);
  } else {
    itr_set_real_mode_handler(irq + 0x8, (itr_handler_real_mode)itr_handle_irq, (void*)irq);
  }
}

#define interrupt __attribute__((interrupt))

typedef struct {
  u16 limit;
  u32 base;
} packed idt_ptr_t;

enum itr_gate_type {
  INTERRUPT_GATE_TYPE_TASK = 0x5,
  INTERRUPT_GATE_TYPE_INTERRUPT = 0xE,
  INTERRUPT_GATE_TYPE_TRAP = 0xF,
};

typedef void interrupt (*base_itr_handler)(itr_frame *frame);

static void set_base_handler(u8 vector, base_itr_handler handler,
                        enum itr_gate_type type) {
  u32 handler_int = (u32)handler;
  idt[vector].offset_low = handler_int & 0xFFFF;
  idt[vector].selector = 0x08;
  idt[vector].zero = 0;
  idt[vector].type_attr = 0x80 | type;
  idt[vector].offset_high = (handler_int >> 16) & 0xFFFF;
}


static void set_base_interrupt_handler(u8 vector, base_itr_handler handler) {
  set_base_handler(vector, handler, INTERRUPT_GATE_TYPE_INTERRUPT);
}

static void set_base_trap_handler(u8 vector, base_itr_handler handler) {
  set_base_handler(vector, handler, INTERRUPT_GATE_TYPE_TRAP);
}



static void print_itr_frame(itr_frame *frame) {
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

#define DEFAULT_INTERRUPT(num)                                                 \
  void interrupt handle_itr_##num(itr_frame *frame) {                          \
    if (itr_handlers[num]) {                                                   \
      itr_handlers[num](num, frame, itr_handler_data[num]);                    \
    } else {                                                                   \
      printf(INTERRUPT_MSG, num);                                              \
    }                                                                          \
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
void interrupt handle_itr_39(itr_frame *frame) {
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


const base_itr_handler default_itr_handlers[256] = {
    [0] = handle_itr_0,     [1] = handle_itr_1,     [2] = handle_itr_2,
    [3] = handle_itr_3,     [4] = handle_itr_4,     [5] = handle_itr_5,
    [6] = handle_itr_6,     [7] = handle_itr_7,     [8] = handle_itr_8,
    [9] = handle_itr_9,     [10] = handle_itr_10,   [11] = handle_itr_11,
    [12] = handle_itr_12,   [13] = handle_itr_13,   [14] = handle_itr_14,
    [15] = handle_itr_15,   [16] = handle_itr_16,   [17] = handle_itr_17,
    [18] = handle_itr_18,   [19] = handle_itr_19,   [20] = handle_itr_20,
    [21] = handle_itr_21,   [22] = handle_itr_22,   [23] = handle_itr_23,
    [24] = handle_itr_24,   [25] = handle_itr_25,   [26] = handle_itr_26,
    [27] = handle_itr_27,   [28] = handle_itr_28,   [29] = handle_itr_29,
    [30] = handle_itr_30,   [31] = handle_itr_31,   [32] = handle_itr_32,
    [33] = handle_itr_33,   [34] = handle_itr_34,   [35] = handle_itr_35,
    [36] = handle_itr_36,   [37] = handle_itr_37,   [38] = handle_itr_38,
    [39] = handle_itr_39,   [40] = handle_itr_40,   [41] = handle_itr_41,
    [42] = handle_itr_42,   [43] = handle_itr_43,   [44] = handle_itr_44,
    [45] = handle_itr_45,   [46] = handle_itr_46,   [47] = handle_itr_47,
    [48] = handle_itr_48,   [49] = handle_itr_49,   [50] = handle_itr_50,
    [51] = handle_itr_51,   [52] = handle_itr_52,   [53] = handle_itr_53,
    [54] = handle_itr_54,   [55] = handle_itr_55,   [56] = handle_itr_56,
    [57] = handle_itr_57,   [58] = handle_itr_58,   [59] = handle_itr_59,
    [60] = handle_itr_60,   [61] = handle_itr_61,   [62] = handle_itr_62,
    [63] = handle_itr_63,   [64] = handle_itr_64,   [65] = handle_itr_65,
    [66] = handle_itr_66,   [67] = handle_itr_67,   [68] = handle_itr_68,
    [69] = handle_itr_69,   [70] = handle_itr_70,   [71] = handle_itr_71,
    [72] = handle_itr_72,   [73] = handle_itr_73,   [74] = handle_itr_74,
    [75] = handle_itr_75,   [76] = handle_itr_76,   [77] = handle_itr_77,
    [78] = handle_itr_78,   [79] = handle_itr_79,   [80] = handle_itr_80,
    [81] = handle_itr_81,   [82] = handle_itr_82,   [83] = handle_itr_83,
    [84] = handle_itr_84,   [85] = handle_itr_85,   [86] = handle_itr_86,
    [87] = handle_itr_87,   [88] = handle_itr_88,   [89] = handle_itr_89,
    [90] = handle_itr_90,   [91] = handle_itr_91,   [92] = handle_itr_92,
    [93] = handle_itr_93,   [94] = handle_itr_94,   [95] = handle_itr_95,
    [96] = handle_itr_96,   [97] = handle_itr_97,   [98] = handle_itr_98,
    [99] = handle_itr_99,   [100] = handle_itr_100, [101] = handle_itr_101,
    [102] = handle_itr_102, [103] = handle_itr_103, [104] = handle_itr_104,
    [105] = handle_itr_105, [106] = handle_itr_106, [107] = handle_itr_107,
    [108] = handle_itr_108, [109] = handle_itr_109, [110] = handle_itr_110,
    [111] = handle_itr_111, [112] = handle_itr_112, [113] = handle_itr_113,
    [114] = handle_itr_114, [115] = handle_itr_115, [116] = handle_itr_116,
    [117] = handle_itr_117, [118] = handle_itr_118, [119] = handle_itr_119,
    [120] = handle_itr_120, [121] = handle_itr_121, [122] = handle_itr_122,
    [123] = handle_itr_123, [124] = handle_itr_124, [125] = handle_itr_125,
    [126] = handle_itr_126, [127] = handle_itr_127, [128] = handle_itr_128,
    [129] = handle_itr_129, [130] = handle_itr_130, [131] = handle_itr_131,
    [132] = handle_itr_132, [133] = handle_itr_133, [134] = handle_itr_134,
    [135] = handle_itr_135, [136] = handle_itr_136, [137] = handle_itr_137,
    [138] = handle_itr_138, [139] = handle_itr_139, [140] = handle_itr_140,
    [141] = handle_itr_141, [142] = handle_itr_142, [143] = handle_itr_143,
    [144] = handle_itr_144, [145] = handle_itr_145, [146] = handle_itr_146,
    [147] = handle_itr_147, [148] = handle_itr_148, [149] = handle_itr_149,
    [150] = handle_itr_150, [151] = handle_itr_151, [152] = handle_itr_152,
    [153] = handle_itr_153, [154] = handle_itr_154, [155] = handle_itr_155,
    [156] = handle_itr_156, [157] = handle_itr_157, [158] = handle_itr_158,
    [159] = handle_itr_159, [160] = handle_itr_160, [161] = handle_itr_161,
    [162] = handle_itr_162, [163] = handle_itr_163, [164] = handle_itr_164,
    [165] = handle_itr_165, [166] = handle_itr_166, [167] = handle_itr_167,
    [168] = handle_itr_168, [169] = handle_itr_169, [170] = handle_itr_170,
    [171] = handle_itr_171, [172] = handle_itr_172, [173] = handle_itr_173,
    [174] = handle_itr_174, [175] = handle_itr_175, [176] = handle_itr_176,
    [177] = handle_itr_177, [178] = handle_itr_178, [179] = handle_itr_179,
    [180] = handle_itr_180, [181] = handle_itr_181, [182] = handle_itr_182,
    [183] = handle_itr_183, [184] = handle_itr_184, [185] = handle_itr_185,
    [186] = handle_itr_186, [187] = handle_itr_187, [188] = handle_itr_188,
    [189] = handle_itr_189, [190] = handle_itr_190, [191] = handle_itr_191,
    [192] = handle_itr_192, [193] = handle_itr_193, [194] = handle_itr_194,
    [195] = handle_itr_195, [196] = handle_itr_196, [197] = handle_itr_197,
    [198] = handle_itr_198, [199] = handle_itr_199, [200] = handle_itr_200,
    [201] = handle_itr_201, [202] = handle_itr_202, [203] = handle_itr_203,
    [204] = handle_itr_204, [205] = handle_itr_205, [206] = handle_itr_206,
    [207] = handle_itr_207, [208] = handle_itr_208, [209] = handle_itr_209,
    [210] = handle_itr_210, [211] = handle_itr_211, [212] = handle_itr_212,
    [213] = handle_itr_213, [214] = handle_itr_214, [215] = handle_itr_215,
    [216] = handle_itr_216, [217] = handle_itr_217, [218] = handle_itr_218,
    [219] = handle_itr_219, [220] = handle_itr_220, [221] = handle_itr_221,
    [222] = handle_itr_222, [223] = handle_itr_223, [224] = handle_itr_224,
    [225] = handle_itr_225, [226] = handle_itr_226, [227] = handle_itr_227,
    [228] = handle_itr_228, [229] = handle_itr_229, [230] = handle_itr_230,
    [231] = handle_itr_231, [232] = handle_itr_232, [233] = handle_itr_233,
    [234] = handle_itr_234, [235] = handle_itr_235, [236] = handle_itr_236,
    [237] = handle_itr_237, [238] = handle_itr_238, [239] = handle_itr_239,
    [240] = handle_itr_240, [241] = handle_itr_241, [242] = handle_itr_242,
    [243] = handle_itr_243, [244] = handle_itr_244, [245] = handle_itr_245,
    [246] = handle_itr_246, [247] = handle_itr_247, [248] = handle_itr_248,
    [249] = handle_itr_249, [250] = handle_itr_250, [251] = handle_itr_251,
    [252] = handle_itr_252, [253] = handle_itr_253, [254] = handle_itr_254,
    [255] = handle_itr_255,
};

void interrupt handle_divide_by_zero(itr_frame *frame) {
  printf("Divide by zero!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_debug(itr_frame *frame) {
  printf("Debug!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_nmi(itr_frame *frame) {
  printf("NMI!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_breakpoint(itr_frame *frame) {
  printf("Breakpoint!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_overflow(itr_frame *frame) {
  printf("Overflow!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_bound_range_exceeded(itr_frame *frame) {
  printf("Bound range exceeded!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_invalid_opcode(itr_frame *frame) {
  printf("Invalid opcode!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_device_not_available(itr_frame *frame) {
  printf("Device not available!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_double_fault(itr_frame *frame) {
  printf("Double fault!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_coprocessor_segment_overrun(itr_frame *frame) {
  printf("Coprocessor segment overrun!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_invalid_tss(itr_frame *frame) {
  printf("Invalid TSS!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_segment_not_present(itr_frame *frame) {
  printf("Segment not present!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_stack_segment_fault(itr_frame *frame) {
  printf("Stack segment fault!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_general_protection_fault(itr_frame *frame) {
  printf("General protection fault!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_page_fault(itr_frame *frame, u32 error_code) {
  printf("Page fault!\n");
  print_itr_frame(frame);
  printf("error code: %x\n", error_code);
  halt();
}

void interrupt handle_x87_floating_point_exception(itr_frame *frame) {
  printf("x87 floating point exception!\n");
  print_itr_frame(frame);
  halt();
}

void interrupt handle_alignment_check(itr_frame *frame, u32 error_code) {
  printf("Alignment check!\n");
  print_itr_frame(frame);
  printf("error code: %x\n", error_code);
  halt();
}

void interrupt handle_machine_check(itr_frame *frame) {
  printf("Machine check!\n");
  print_itr_frame(frame);
}

void interrupt handle_simd_floating_point_exception(itr_frame *frame) {
  printf("SIMD floating point exception!\n");
  print_itr_frame(frame);
}

void interrupt handle_virtualization_exception(itr_frame *frame) {
  printf("Virtualization exception!\n");
  print_itr_frame(frame);
}

void interrupt handle_security_exception(itr_frame *frame) {
  printf("Security exception!\n");
  print_itr_frame(frame);
}

static itr_handler_real_mode real_mode_irt_handlers[256] = {};
static void** real_mode_itr_data[256] = {};

void itr_reload_idt(void) {
  idt_ptr_t idt_ptr;
  idt_ptr.limit = sizeof(idt_entry) * 256 - 1;
  idt_ptr.base = (u32)&idt;
  // Make sure the PIC is mapped
  asm volatile("lidt %0" : : "m"(idt_ptr));
}

void initialize_pic_protected_mode(u8 master_mask, u8 slave_mask) {
  // Remap the PIC to protected mode
  outb(0x20, 0x11); // begin initialization
  outb(0xA0, 0x11);
  outb(0x21, 0x20); // master offset
  outb(0xA1, 0x28); // slave offset
  outb(0x21, 0x04); // master/slave wiring
  outb(0xA1, 0x02);
  outb(0x21, 0x01); // 8088
  outb(0xA1, 0x01);
  outb(0x21, master_mask);
  outb(0xA1, slave_mask);
}

void itr_init(void) {
  asm volatile("cli");
  for (int i = 0; i < 256; i++) {
    set_base_interrupt_handler(i, default_itr_handlers[i]);
  }
  set_base_trap_handler(0, handle_divide_by_zero);
  set_base_trap_handler(1, handle_debug);
  set_base_interrupt_handler(2, handle_nmi);
  set_base_trap_handler(3, handle_breakpoint);
  set_base_trap_handler(4, handle_overflow);
  set_base_trap_handler(5, handle_bound_range_exceeded);
  set_base_trap_handler(6, handle_invalid_opcode);
  set_base_trap_handler(7, handle_device_not_available);
  set_base_interrupt_handler(8, handle_double_fault);
  set_base_trap_handler(9, handle_coprocessor_segment_overrun);
  set_base_trap_handler(10, handle_invalid_tss);
  set_base_trap_handler(11, handle_segment_not_present);
  set_base_trap_handler(12, handle_stack_segment_fault);
  set_base_trap_handler(13, handle_general_protection_fault);
  set_base_trap_handler(14, (void *)handle_page_fault);
  set_base_trap_handler(16, handle_x87_floating_point_exception);
  set_base_trap_handler(17, (void *)handle_alignment_check);
  set_base_interrupt_handler(18, (void *)handle_machine_check);
  set_base_trap_handler(19, handle_simd_floating_point_exception);
  set_base_trap_handler(20, handle_virtualization_exception);
  set_base_trap_handler(30, handle_security_exception);
  initialize_pic_protected_mode(0xFF, 0xFF);
  itr_reload_idt();
  printf("Interrupts initialized\n");
}

void itr_clear_handler(u8 vector) {
  itr_set_handler(vector, NULL, NULL);
}

void itr_enable(void) {
  printf("Enabling interrupts\n");
  outb(0x21, 0x00);
  outb(0xA1, 0x00);
  asm volatile("sti");
}

void itr_disable(void) {
  printf("Disabling interrupts\n");
  outb(0x21, 0xFF);
  outb(0xA1, 0xFF);
  asm volatile("cli");
}

bool itr_enabled() {
  u32 flags;
  // Query the interrupt flag
  asm volatile("pushf\n\t"
               "pop %0"
               : "=g"(flags));
  return flags & (1 << 9);
}

void itr_set_real_mode_handler(u8 vector, itr_handler_real_mode handler,
                               void *data) {
  real_mode_irt_handlers[vector] = handler;
  real_mode_itr_data[vector] = data;
}

static void print_real_mode_stack_frame(itr_frame_real_mode* frame) {
      printf("ip: %x\n", frame->ip);
      printf("cs: %x\n", frame->cs);
      printf("(eip): %x\n", (frame->cs << 4) + frame->ip);
      printf("flags: %x\n", frame->flags);
      printf("eax: %x\n", frame->eax);
      printf("ebx: %x\n", frame->ebx);
      printf("ecx: %x\n", frame->ecx);
      printf("edx: %x\n", frame->edx);
      printf("esi: %x\n", frame->esi);
      printf("edi: %x\n", frame->edi);
      printf("ebp: %x\n", frame->ebp);
      printf("ds: %x\n", frame->ds);
      printf("es: %x\n", frame->es);
      printf("ss: %x\n", frame->ss);
      printf("esp: %x\n", frame->esp);
      printf("fs: %x\n", frame->fs);
      printf("gs: %x\n", frame->gs);
}

void initialize_pic_real_mode(u8 master_mask, u8 slave_mask) {
  asm volatile("cli");
  // Remap the PIC for real mode
  outb(0x20, 0x11); // begin initialization
  outb(0xA0, 0x11);
  outb(0x21, 0x08); // master offset
  outb(0xA1, 0x70); // slave offset
  outb(0x21, 0x04); // master/slave wiring
  outb(0xA1, 0x02);
  outb(0x21, 0x01); // 8088
  outb(0xA1, 0x01);
  outb(0x21, master_mask);
  outb(0xA1, slave_mask);
}

void itr_setup_real_mode() {
  initialize_pic_real_mode(0x00, 0x00);
}

void itr_real_mode_interrupt(void) {
  u8 *_frame;
  asm volatile("mov %%ebp, %0" : "=g"(_frame));
  _frame += 8; // advance by 8 to account for EIP and EBP pushed on the stack
  itr_frame_real_mode *frame = (itr_frame_real_mode *)(_frame);
  // Copy IP, CS, and flags from the old stack
  u32 esp = *(u32*)(0x9c000);
  u16 ss = *(u16*)(0x9c004);
  u32 prev_stack = (ss << 4) + (esp & 0xFFFF);
  frame->esp = esp;
  frame->ss = ss;
  // Check the order here against interrupt.asm where these are
  // pushed on the previous stack
  frame->ebp = *(u32*)(prev_stack);
  frame->gs = *(u16*)(prev_stack + 4);
  u16 itr_vector = *(u16*)(prev_stack + 6);
  frame->ip = *(u16*)(prev_stack + 8);
  frame->cs = *(u16*)(prev_stack + 10);
  frame->flags = *(u16*)(prev_stack + 12);
  if (frame->ebp & 0xFFFF0000) {
    // TODO Figure out why this happens, somehow the stack is getting corrupted
    // and the high word of ebp is using the 9 clearly from our stack
    frame->ebp = frame->ebp & 0xFFFF;
  }
  // Save the PIC state
  //u8 unused pic_state[2];
  //pic_state[0] = inb(0x21);
  //pic_state[1] = inb(0xA1);
  // Setup interrupts in protected mode
  // TODO this doesn't properly restore
  // the PIC state and breaks DPMI
  //initialize_pic_protected_mode(0x00, 0x00);
  itr_reload_idt();
  if (real_mode_irt_handlers[itr_vector] != NULL) {
    void *data = real_mode_itr_data[itr_vector];
    real_mode_irt_handlers[itr_vector](itr_vector, frame, data);
  } else {
    if (itr_vector < 0x10) {
      printf("Exception 0x%x\n", itr_vector);
      print_real_mode_stack_frame(frame);
      for (;;) {
        asm("xchg %bx, %bx");
        asm volatile("hlt");
      }
    }
    else if (itr_vector >= 0x20 && itr_vector <= 0x2F) {
      printf("unhandled irq %d\n", itr_vector - 0x20);
    } else {
      printf("Got unknown real mode interrupt %x\n", itr_vector);
      // printf("Got real mode interrupt %x\n", itr_vector);
      print_real_mode_stack_frame(frame);
    }
  }
  // Copy the registers back to the old stack
  *(u32*)(prev_stack) = frame->ebp;
  *(u16*)(prev_stack + 4) = frame->gs;
  *(u16*)(prev_stack + 8) = frame->ip;
  *(u16*)(prev_stack + 10) = frame->cs;
  *(u16*)(prev_stack + 12) = frame->flags;
  // Restore the PIC state
  //initialize_pic_real_mode(pic_state[0], pic_state[1]);
}
