//Projeto: Relógio
module Watch (CLK_IN, RST, N_F, S_HM, DISPLAY, DISPLAY_EN1, DISPLAY_EN2, DISPLAY_EN3, DISPLAY_EN4);

//Declaração de ports
input 			CLK_IN;
input 			S_HM;
input 			N_F;
input 			RST;
output [6:0] 	DISPLAY;
output 		 	DISPLAY_EN1;
output 		 	DISPLAY_EN2;
output 		 	DISPLAY_EN3;
output 		 	DISPLAY_EN4;

//Declaração de tipos de dados
wire 			clk_sel;
wire 			clk_sel_normal;
wire 			clk_sel_fast;
wire 			clk_display;
wire 			rst;
wire	[3:0]	display_s0;
wire	[6:0]	Display_s0;
wire			En_S0;
wire 			seg1;
wire	[3:0]	display_s1;
wire	[6:0]	Display_s1;
wire			En_S1;
wire 			min0;
wire	[3:0]	display_m0;
wire	[6:0]	Display_m0;
wire			En_M0;
wire 			min1;
wire	[3:0]	display_m1;
wire	[6:0]	Display_m1;
wire			En_M1;
wire 			hor0;
wire	[3:0]	display_h0;
wire	[6:0]	Display_h0;
wire			En_H0;
wire 			hor1;
wire	[3:0]	display_h1;
wire	[6:0]	Display_h1;
wire			En_H1;


//Funcionamento do circuito
// Ligando Prescaler
Prescaler pr1 (
	.clk_in(CLK_IN),
	.rst(RST),
	.clk_1Hz(clk_sel_normal),
	.clk_c(clk_sel_fast),	
	.clk_100Hz(clk_display)
	);

// Ligando Mux Prescaler
Mux_Prescaler mx1 (
	.SEL(N_F),
	.CLK_NORMAL(clk_sel_normal),
	.CLK_FAST(clk_sel_fast),
	.CLK_REL(clk_sel),	
	);
	
// Ligando Mux Display
Mux_Display mx2 (
	.CLK_IN(clk_display),
	.S_HM(S_HM),
	.DISPLAY_S0(Display_s0),
	.EN_S0(En_S0),
	.DISPLAY_S1(Display_s1),
	.EN_S1(En_S1),
	.DISPLAY_M0(Display_m0),
	.EN_M0(En_M0),
	.DISPLAY_M1(Display_m1),
	.EN_M1(En_M1),
	.DISPLAY_H0(Display_h0),
	.EN_H0(En_H0),
	.DISPLAY_H1(Display_h1),
	.EN_H1(En_H1),	
	.SEG_A(DISPLAY[0]), 
	.SEG_B(DISPLAY[1]), 
	.SEG_C(DISPLAY[2]), 
	.SEG_D(DISPLAY[3]), 
	.SEG_E(DISPLAY[4]), 
	.SEG_F(DISPLAY[5]), 
	.SEG_G(DISPLAY[6]),
	.EN_1(DISPLAY_EN1), 
	.EN_2(DISPLAY_EN2), 
	.EN_3(DISPLAY_EN3), 
	.EN_4(DISPLAY_EN4)
	);

// Primeiro Dígito do Segundo
	
// Ligando Contador_0 ~ 9 S0
Counter_9 ct1 (
	.CLK_IN(clk_sel),
	.BCD_A(display_s0[0]),
	.BCD_B(display_s0[1]),
	.BCD_C(display_s0[2]),
	.BCD_D(display_s0[3]),	
	.CLK_OUT(seg1),
	.RST(rst)
	);

// Ligando Display S0	
Display ds1 (
	.A(display_s0[0]),
	.B(display_s0[1]),
	.C(display_s0[2]),
	.D(display_s0[3]),
	.SEG_A(Display_s0[0]),
	.SEG_B(Display_s0[1]),
	.SEG_C(Display_s0[2]),
	.SEG_D(Display_s0[3]),
	.SEG_E(Display_s0[4]),
	.SEG_F(Display_s0[5]),
	.SEG_G(Display_s0[6]),
	.EN_4(En_S0)
	);

// Segundo Dígito do Segundo
	
// Ligando Contador_0 ~ 5 S1
Counter_5 ct2 (
	.CLK_IN(seg1),
	.BCD_A(display_s1[0]),
	.BCD_B(display_s1[1]),
	.BCD_C(display_s1[2]),
	.BCD_D(display_s1[3]),
	.CLK_OUT(min0),
	.RST(rst)	
	);

// Ligando Display S1
Display ds2 (
	.A(display_s1[0]),
	.B(display_s1[1]),
	.C(display_s1[2]),
	.D(display_s1[3]),
	.SEG_A(Display_s1[0]),
	.SEG_B(Display_s1[1]),
	.SEG_C(Display_s1[2]),
	.SEG_D(Display_s1[3]),
	.SEG_E(Display_s1[4]),
	.SEG_F(Display_s1[5]),
	.SEG_G(Display_s1[6]),
	.EN_4(En_S1)
	);

// Primeiro Dígito do Minuto	
	
// Ligando Contador_0 ~ 9 M0
Counter_9 ct3 (
	.CLK_IN(min0),
	.BCD_A(display_m0[0]),
	.BCD_B(display_m0[1]),
	.BCD_C(display_m0[2]),
	.BCD_D(display_m0[3]),
	.CLK_OUT(min1),
	.RST(rst)	
	);

// Ligando Display M0
Display ds3 (
	.A(display_m0[0]),
	.B(display_m0[1]),
	.C(display_m0[2]),
	.D(display_m0[3]),
	.SEG_A(Display_m0[0]),
	.SEG_B(Display_m0[1]),
	.SEG_C(Display_m0[2]),
	.SEG_D(Display_m0[3]),
	.SEG_E(Display_m0[4]),
	.SEG_F(Display_m0[5]),
	.SEG_G(Display_m0[6]),
	.EN_4(En_M0)
	);

// Segundo Dígito do Minuto	
	
// Ligando Contador_0 ~ 5 M1
Counter_5 ct4 (
	.CLK_IN(min1),
	.BCD_A(display_m1[0]),
	.BCD_B(display_m1[1]),
	.BCD_C(display_m1[2]),
	.BCD_D(display_m1[3]),
	.CLK_OUT(hor0),
	.RST(rst)	
	);

// Ligando Display M1
Display ds4 (
	.A(display_m1[0]),
	.B(display_m1[1]),
	.C(display_m1[2]),
	.D(display_m1[3]),
	.SEG_A(Display_m1[0]),
	.SEG_B(Display_m1[1]),
	.SEG_C(Display_m1[2]),
	.SEG_D(Display_m1[3]),
	.SEG_E(Display_m1[4]),
	.SEG_F(Display_m1[5]),
	.SEG_G(Display_m1[6]),
	.EN_4(En_M1)
	);

// Primeiro Dígito da hora	
	
// Ligando Contador_0 ~ 9 H0
Counter_9 ct5 (
	.CLK_IN(hor0),
	.BCD_A(display_h0[0]),
	.BCD_B(display_h0[1]),
	.BCD_C(display_h0[2]),
	.BCD_D(display_h0[3]),
	.CLK_OUT(hor1),
	.RST(rst)	
	);

// Ligando Display M0
Display ds5 (
	.A(display_h0[0]),
	.B(display_h0[1]),
	.C(display_h0[2]),
	.D(display_h0[3]),
	.SEG_A(Display_h0[0]),
	.SEG_B(Display_h0[1]),
	.SEG_C(Display_h0[2]),
	.SEG_D(Display_h0[3]),
	.SEG_E(Display_h0[4]),
	.SEG_F(Display_h0[5]),
	.SEG_G(Display_h0[6]),
	.EN_4(En_H0)
	);

// Segundo Dígito da Hora	
	
// Ligando Contador_0 ~ 5 H1
Counter_5 ct6 (
	.CLK_IN(hor1),
	.BCD_A(display_h1[0]),
	.BCD_B(display_h1[1]),
	.BCD_C(display_h1[2]),
	.BCD_D(display_h1[3]),
	.RST(rst)
	);

// Ligando Display M1
Display ds6 (
	.A(display_h1[0]),
	.B(display_h1[1]),
	.C(display_h1[2]),
	.D(display_h1[3]),
	.SEG_A(Display_h1[0]),
	.SEG_B(Display_h1[1]),
	.SEG_C(Display_h1[2]),
	.SEG_D(Display_h1[3]),
	.SEG_E(Display_h1[4]),
	.SEG_F(Display_h1[5]),
	.SEG_G(Display_h1[6]),
	.EN_4(En_H1)
	);

// Ligando Reset 24h
Reset_24 rs1 (
	.CLK(clk_sel), 
	.BCD_H0(display_h0), 
	.BCD_H1(display_h1), 
	.BCD_M0(display_m0), 
	.BCD_M1(display_m1), 
	.BCD_S0(display_s0), 
	.BCD_S1(display_s1), 
	.RST(rst)
	);
	
endmodule