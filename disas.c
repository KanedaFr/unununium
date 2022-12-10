// Copyright 2008-2010  Segher Boessenkool  <segher@kernel.crashing.org>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#include "disas.h"


static const char *regs[] = { "sp", "r1", "r2", "r3", "r4", "bp", "sr", "pc" };

static const char *jumps[] = {
	"jb", "jae", "jge", "jl", "jne", "je", "jpl", "jmi",
	"jbe", "ja", "jle", "jg", "jvc", "jvs", "jmp"
};

static void print_alu_op_start(u8 op0, u8 opA)
{
	static const char *alu_op_start[] = {
		"%s += ", "%s += ", "%s -= ", "%s -= ",
		"cmp %s, ", "<BAD>", "%s =- ", "<BAD>",
		"%s ^= ", "%s = ", "%s |= ", "%s &= ",
		"test %s, "
	};

	printf(alu_op_start[op0], regs[opA]);
}

static void print_alu_op3(u8 op0, u8 opB)
{
	static const char *alu_op3[] = {
		"%s + ", "%s + ", "%s - ", "%s - ",
		"cmp %s, ", "<BAD>", "-", "<BAD>",
		"%s ^ ", "", "%s | ", "%s & ",
		"test %s, "
	};

	printf(alu_op3[op0], regs[opB]);
}

static void print_alu_op_end(u8 op0)
{
	if (op0 == 1 || op0 == 3)
		printf(", carry");
	printf("\n");
}

static void print_indirect_op(u8 opN, u8 opB)
{
	const char *forms[] = { "[%s]", "[%s--]", "[%s++]", "[++%s]" };

	if (opN & 4)
		printf("ds:");
	printf(forms[opN & 3], regs[opB]);
}

char bufferString[256];	

static char *getPPULayerRegisterName(u8 layerID, u8 registerID){
	static const char *registers[] = {
		"SCROLL_X", "SCROLL_Y", "ATTR", "CTRL", "TILE_ADDR ","ATTR_ADDR "
	};
	
	sprintf(bufferString, "PPU_BG%d_%s", layerID+1, registers[registerID]);
	return bufferString;
}
static char *getPPUSpriteRegisterName(u8 spriteID, u8 registerID){
	static const char *registers[] = {
		"TILE", "X", "Y", "ATTR"
	};
	
	sprintf(bufferString, "PPU_SPRITE_%s(%03d)", registers[registerID], spriteID);
	return bufferString;
}
static char *getPPUName(u16 i){
	if ((i>=0x2810) && (i<0x2816)){
		return getPPULayerRegisterName(0, i-0x2810);
	}
	else if ((i>=0x2816) && (i<0x281c)){
		return getPPULayerRegisterName(1, i-0x2816);
	}
	else if ((i>=0x2900) && (i<0x2A00)){
		sprintf(bufferString, "PPU_LINE_SCROLL(%03d)", i-0x2900);
		return bufferString;
	}	
	else if ((i>=0x2A00) && (i<0x2B00)){
		sprintf(bufferString, "PPU_LINE_COMPRESS(%03d)", i-0x2A00);
		return bufferString;
	}
	else if ((i>=0x2B00) && (i<0x2C00)){
		sprintf(bufferString, "PPU_COLOR(%03d)", i-0x2B00);
		return bufferString;
	}
	else if ((i>=0x2C00) && (i<0x3000)){
		return getPPUSpriteRegisterName((i - 0x2C00)/4, i%4);
	}
	

	switch (i)
	{
	case 0x281C:
		sprintf(bufferString, "PPU_VERT_COMPRESS_AMOUNT");
		break;
	case 0x281D:
		sprintf(bufferString, "PPU_VERT_COMPRESS_OFFSET");
		break;
	case 0x2820:
		sprintf(bufferString, "PPU_BG1_SEGMENT_ADDR");
		break;
	case 0x2821:
		sprintf(bufferString, "PPU_BG2_SEGMENT_ADDR");
		break;
	case 0x2822:
		sprintf(bufferString, "PPU_SPRITE_SEGMENT_ADDR");
		break;
	case 0x282A:
		sprintf(bufferString, "PPU_BLEND_LEVEL"); // (2 bits)");
		break;
	//case 0x282B:case 0x282C:case 0x282D:case 0x282E:case 0x282F:
	//	sprintf(bufferString, "PPU_%04x", i);
		break;
	case 0x2830:
		sprintf(bufferString, "PPU_FADE_CTRL"); // (8 bits)");
		break;
	//case 0x2831:case 0x2832:case 0x2833:case 0x2834:case 0x2835:
	//	sprintf(bufferString, "PPU_%04x", i);
	//	break;
	case 0x2836:
		sprintf(bufferString, "PPU_IRQ_POS_Y"); // (9 bits)");
		break;
	case 0x2837:
		sprintf(bufferString, "PPU_IRQ_POS_X"); // (9 bits)");
		break;
	case 0x2838:
		sprintf(bufferString, "PPU_CURRENT_LINE");
		break;
	case 0x2839:
		sprintf(bufferString, "PPU_LIGHTPEN_LATCH_1ST_LINE");
		break;
	//case 0x283A:case 0x283B:
	//	sprintf(bufferString, "PPU_%04x", i);
	//	break;
	case 0x283C:
		sprintf(bufferString, "PPU_TV_CTRL1");
		break;
	case 0x283D:
		sprintf(bufferString, "PPU_TV_CTRL2"); 
		break;
	case 0x283E:
		sprintf(bufferString, "PPU_LIGHTPEN_Y");// (9 bits)");
		break;
	case 0x283F:
		sprintf(bufferString, "PPU_LIGHTPEN_X"); // (9 bits)");
		break;
	//case 0x2840:case 0x2841:
	//	sprintf(bufferString, "PPU_%04x", i);
	//	break;
	case 0x2842:
		sprintf(bufferString, "PPU_SPRITE_CTRL");
		break;
	//case 0x2843...0x2853:
	//	sprintf(bufferString, "PPU_%04x", i);
	//	break;
	case 0x2854:
		sprintf(bufferString, "PPU_STN_LCD_CTRL"); // (bits 4 and 5: framerate, bit 3: CkvSel, bit 2, resolution, bits 1 and 0: color mode)");
		break;
	//case 0x2855...0x2861:
	//	sprintf(bufferString, "PPU_%04x", i);
	//	break;
	case 0x2862:
		sprintf(bufferString, "PPU_IRQ_ENABLE");// (bit 2: DMA, bit 1: VDO, bit 0: blanking)");
		break;
	case 0x2863:
		sprintf(bufferString, "PPU_IRQ_STATUS");// (same layout)");
		break;
	//case 0x2864...0x286F:
	//	sprintf(bufferString, "PPU_%04x", i);
	//	break;
	case 0x2870:
		sprintf(bufferString, "PPU_SPRITE_DMA_SRC");
		break;
	case 0x2871:
		sprintf(bufferString, "PPU_SPRITE_DMA_DEST");
		break;
	case 0x2872:
		sprintf(bufferString, "PPU_SPRITE_DMA_LEN");
		break;
	default:
		fprintf(stderr,"Unknow access to PPU_%04x\n", i);
		sprintf(bufferString, "PPU_%04x", i);
		break;
	}


	return bufferString;
}
static char *getAddressName(u16 i){
	if (!isVsmile)	{
		sprintf(bufferString, "%04x", i);
		return bufferString;
	}

	sprintf(bufferString, "<bad>");

	if (i < 0x2800)
	{
		sprintf(bufferString, "%04x", i);
	}
	else if (i < 0x3000){
		return getPPUName(i);
	}
	else if (i < 0x3800){
		sprintf(bufferString, "SPU_%04x", i);
	}
	else if (i < 0x3d00){
		//TODO : issue error ?
		sprintf(bufferString, "%04x", i);
	}
	else if (i < 0x3e04){
		sprintf(bufferString, "IO_%04x", i);
	}
	else
		sprintf(bufferString, "%04x", i);

	return bufferString;
}
u32 disas(const u16 *mem, u32 offset)
{
	u8 op0, opA, op1, opN, opB, opimm;
	u16 op, ximm = 0x0bad;
	u32 len = 1;

	printf("%04x: ", offset);

	op = mem[offset++];


	// the top four bits are the alu op or the branch condition, or E or F
	op0 = (op >> 12);

	// the next three are usually the destination register
	opA = (op >> 9) & 7;

	// and the next three the addressing mode
	op1 = (op >> 6) & 7;

	// the next three can be anything
	opN = (op >> 3) & 7;

	// and the last three usually the second register (source register)
	opB = op & 7;

	// the last six sometimes are a single immediate number
	opimm = op & 63;


	// some insns need a second word:
	if ((op0 < 14 && op1 == 4 && (opN == 1 || opN == 2 || opN == 3))
	 || (op0 == 15 && (op1 == 1 || op1 == 2))) {
		ximm = mem[offset++];
		if (verbose)	printf("%04x %04x   ", op, ximm);
		len = 2;
	} else if (verbose)
		printf("%04x        ", op);

	if (verbose)
		printf("%x %x %x %x %x   ", op0, opA, op1, opN, opB);


	// all-zero and all-one are invalid insns:
	if (op == 0 || op == 0xffff) {
		printf("--\n");
		return 1;
	}


	// first, check for the conditional branch insns
	if (op0 < 15 && opA == 7 && op1 == 0) {
		printf("%s %04x\n", jumps[op0], offset+opimm);
		return 1;
	}
	if (op0 < 15 && opA == 7 && op1 == 1) {
		printf("%s %04x\n", jumps[op0], offset-opimm);
		return 1;
	}


	switch ((op1 << 4) | op0) {

	case 0x05: case 0x15: case 0x25: case 0x35:
	case 0x45: case 0x55: case 0x65: case 0x75:
	case 0x85: case 0x95: case 0xa5: case 0xb5:
	case 0xc5: case 0xd5:
	case 0x07: case 0x17: case 0x27: case 0x37:
	case 0x47: case 0x57: case 0x67: case 0x77:
	case 0x87: case 0x97: case 0xa7: case 0xb7:
	case 0xc7: case 0xd7:
	case 0x1d: case 0x5d: case 0x6d:
	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x24: case 0x26: case 0x28: case 0x2a:
	case 0x2b: case 0x2c:
	bad:
		printf("<BAD>\n");
		return len;


	// alu, base+displacement
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x06: case 0x08: case 0x09:
	case 0x0a: case 0x0b: case 0x0c:
		print_alu_op_start(op0, opA);
		printf("[bp+%02x]", opimm);
		print_alu_op_end(op0);
		return 1;
	case 0x0d:
		printf("[bp+%02x] = %s\n", opimm, regs[opA]);
		return 1;


	// alu, 6-bit immediate
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x16: case 0x18: case 0x19:
	case 0x1a: case 0x1b: case 0x1c:
		print_alu_op_start(op0, opA);
		printf("%02x", opimm);
		print_alu_op_end(op0);
		return 1;


	// pop insns
	case 0x29:
		if (op == 0x9a90)
			printf("retf\n");
		else if (op == 0x9a98)
			printf("reti\n");
		else if (opA+1 < 8 && opA+opN < 8)
			printf("pop %s, %s from [%s]\n",
			       regs[opA+1], regs[opA+opN], regs[opB]);
		else
			goto bad;
		return 1;


	// push insns
	case 0x2d:
		if (opA+1 >= opN && opA < opN+7)
			printf("push %s, %s to [%s]\n",
			       regs[opA+1-opN], regs[opA], regs[opB]);
		else
			goto bad;
		return 1;


	// alu, indirect memory
	case 0x30: case 0x31: case 0x32: case 0x33:
	case 0x34: case 0x36: case 0x38: case 0x39:
	case 0x3a: case 0x3b: case 0x3c:
		print_alu_op_start(op0, opA);
		print_indirect_op(opN, opB);
		print_alu_op_end(op0);
		return 1;
	case 0x3d:
		print_indirect_op(opN, opB);
		printf(" = %s\n", regs[opA]);
		return 1;


	case 0x40: case 0x41: case 0x42: case 0x43:
	case 0x44: case 0x46: case 0x48: case 0x49:
	case 0x4a: case 0x4b: case 0x4c:
		switch (opN) {

		// alu, register
		case 0:
			print_alu_op_start(op0, opA);
			printf("%s", regs[opB]);
			print_alu_op_end(op0);
			return 1;

		// alu, 16-bit immediate
		case 1:
			if ((op0 == 4 || op0 == 12 || op0 == 6 || op0 == 9)
			   && opA != opB)
				goto bad;
			if (op0 != 4 && op0 != 12)
				printf("%s = ", regs[opA]);
			print_alu_op3(op0, opB);
			if ((opA>0) && (opA < 5))
				printf("%s", getAddressName(ximm)); //only if r1...r4
			else
				printf("%04x", ximm);
			print_alu_op_end(op0);
			return 2;

		// alu, direct memory
		case 2:
			if ((op0 == 4 || op0 == 12 || op0 == 6 || op0 == 9)
			   && opA != opB)
				goto bad;
			if (op0 != 4 && op0 != 12)
				printf("%s = ", regs[opA]);
			print_alu_op3(op0, opB);
			printf("[%04x]", ximm);
			print_alu_op_end(op0);
			return 2;

		// alu, direct memory
		case 3:
			if (op0 == 4 || op0 == 12)
				goto bad;
			if ((op0 == 6 || op0 == 9) && opA != opB)
				goto bad;
			printf("[%04x] = ", ximm);
			print_alu_op3(op0, opB);
			printf("%s", regs[opA]);
			print_alu_op_end(op0);
			return 2;

		// alu, with shift
		default:
			print_alu_op_start(op0, opA);
			printf("%s asr %x", regs[opB], (opN & 3) + 1);
			print_alu_op_end(op0);
			return 1;
		}

	case 0x4d:
		switch (opN) {

		// alu, direct memory
		case 3:
			if (opA != opB)
				goto bad;
			printf("[%s] = %s\n", getAddressName(ximm), regs[opB]);
			return 2;
		default:
			goto bad;
		}


	// alu, with shift
	case 0x50: case 0x51: case 0x52: case 0x53:
	case 0x54: case 0x56: case 0x58: case 0x59:
	case 0x5a: case 0x5b: case 0x5c:
		print_alu_op_start(op0, opA);
		if ((opN & 4) == 0)
			printf("%s lsl %x", regs[opB], (opN & 3) + 1);
		else
			printf("%s lsr %x", regs[opB], (opN & 3) + 1);
		print_alu_op_end(op0);
		return 1;


	// alu, with shift
	case 0x60: case 0x61: case 0x62: case 0x63:
	case 0x64: case 0x66: case 0x68: case 0x69:
	case 0x6a: case 0x6b: case 0x6c:
		print_alu_op_start(op0, opA);
		if ((opN & 4) == 0)
			printf("%s rol %x", regs[opB], (opN & 3) + 1);
		else
			printf("%s ror %x", regs[opB], (opN & 3) + 1);
		print_alu_op_end(op0);
		return 1;


	// alu, direct memory
	case 0x70: case 0x71: case 0x72: case 0x73:
	case 0x74: case 0x76: case 0x78: case 0x79:
	case 0x7a: case 0x7b: case 0x7c:
		print_alu_op_start(op0, opA);
		printf("[%02x]", opimm);
		print_alu_op_end(op0);
		return 1;
	case 0x7d:
		printf("[%02x] = %s\n", opimm, regs[opA]);
		return 1;


	case 0x1f:
		if (opA == 0) {
			printf("call %04x\n", (opimm << 16) | ximm);
			return 2;
		}
		goto dunno;

	case 0x2f: case 0x3f: case 0x6f: case 0x7f:
		if (opA == 7 && op1 == 2) {
			printf("goto %04x\n", (opimm << 16) | ximm);
			return 2;
		}
		if (opA == 7 && op1 == 3)
			goto dunno;
		goto dunno;


	case 0x0f:
		switch (opN) {
		case 1:
			if (opA == 7)
				goto dunno;
			printf("mr = %s*%s, us\n", regs[opA], regs[opB]);
			return 1;
		default:
			goto dunno;
		}

	case 0x4f:
		switch (opN) {
		case 1:
			if (opA == 7)
				goto dunno;
			printf("mr = %s*%s\n", regs[opA], regs[opB]);
			return 1;
		default:
			goto dunno;
		}

	case 0x5f:
		if (opA != 0)
			goto dunno;
		switch (opimm) {
		case 0x00:
			printf("int off\n");
			return 1;
		case 0x01:
			printf("int irq\n");
			return 1;
		case 0x02:
			printf("int fiq\n");
			return 1;
		case 0x03:
			printf("int fiq,irq\n");
			return 1;
		case 0x04:
			printf("fir_mov on\n");
			return 1;
		case 0x05:
			printf("fir_mov off\n");
			return 1;
		case 0x08:
			printf("irq off\n");
			return 1;
		case 0x09:
			printf("irq on\n");
			return 1;
		case 0x0c:
			printf("fiq off\n");
			return 1;
		case 0x0e:
			printf("fiq on\n");
			return 1;
		case 0x25:
			printf("nop\n");
			return 1;
		default:
			goto dunno;
		}

	case 0x0e: case 0x1e: case 0x2e: case 0x3e:
	case 0x4e: case 0x5e: case 0x6e: case 0x7e:
	dunno:
		printf("<DUNNO>\n");
		return len;

	default:
		printf("<UH-OH, MY MISTAKE.  UNHANDLED, SORRY ABOUT THAT>");
		return len;
	}
}
