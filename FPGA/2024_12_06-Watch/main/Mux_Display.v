
module Mux_Display (SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, EN_1, EN_2, EN_3, EN_4,
						  DISPLAY_S0, DISPLAY_S1, DISPLAY_M0, DISPLAY_M1, DISPLAY_H0, DISPLAY_H1, EN_S0, EN_S1, EN_M0, EN_M1, EN_H0, EN_H1,
						  S_HM, CLK_IN);


	input			[6:0]	DISPLAY_S0;
	input			[6:0]	DISPLAY_S1;
	input			[6:0]	DISPLAY_M0;
	input			[6:0]	DISPLAY_M1;
	input			[6:0]	DISPLAY_H0;
	input			[6:0]	DISPLAY_H1;
	input 				S_HM;
	input 				CLK_IN;
	input 				EN_S0;
	input 				EN_S1;
	input 				EN_M0;
	input 				EN_M1;
	input 				EN_H0;
	input 				EN_H1;
	output reg			SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G;
	output reg			EN_1, EN_2, EN_3, EN_4;
	

	reg	[1:0]	count = 0;

	always @ (posedge CLK_IN)
	begin
		if(~S_HM)
			begin
				if(count == 2'd0)
					begin
						EN_1 <= EN_S0;
						EN_2 <= 1;
						EN_3 <= 1;
						EN_4 <= 1;
						SEG_A	<= DISPLAY_S0[0];
						SEG_B	<= DISPLAY_S0[1];
						SEG_C	<= DISPLAY_S0[2];
						SEG_D	<= DISPLAY_S0[3];
						SEG_E	<= DISPLAY_S0[4];
						SEG_F	<= DISPLAY_S0[5];
						SEG_G	<= DISPLAY_S0[6];
						count <= count + 1;
					end
				else if(count == 2'd1)
					begin
						EN_1 <= 1;
						EN_2 <= EN_S1;
						EN_3 <= 1;
						EN_4 <= 1;
						SEG_A	<= DISPLAY_S1[0];
						SEG_B	<= DISPLAY_S1[1];
						SEG_C	<= DISPLAY_S1[2];
						SEG_D	<= DISPLAY_S1[3];
						SEG_E	<= DISPLAY_S1[4];
						SEG_F	<= DISPLAY_S1[5];
						SEG_G	<= DISPLAY_S1[6];
						count <= 0;
					end
			end

		else if(S_HM)
			begin
				if(count == 2'd0)
					begin
						EN_1 <= EN_M0;
						EN_2 <= 1;
						EN_3 <= 1;
						EN_4 <= 1;
						SEG_A	<= DISPLAY_M0[0];
						SEG_B	<= DISPLAY_M0[1];
						SEG_C	<= DISPLAY_M0[2];
						SEG_D	<= DISPLAY_M0[3];
						SEG_E	<= DISPLAY_M0[4];
						SEG_F	<= DISPLAY_M0[5];
						SEG_G	<= DISPLAY_M0[6];
						count <= count + 1;
					end
				else if(count == 2'd1)
					begin
						EN_1 <= 1;
						EN_2 <= EN_M1;
						EN_3 <= 1;
						EN_4 <= 1;
						SEG_A	<= DISPLAY_M1[0];
						SEG_B	<= DISPLAY_M1[1];
						SEG_C	<= DISPLAY_M1[2];
						SEG_D	<= DISPLAY_M1[3];
						SEG_E	<= DISPLAY_M1[4];
						SEG_F	<= DISPLAY_M1[5];
						SEG_G	<= DISPLAY_M1[6];
						count <= count + 1;
					end
				else if(count == 2'd2)
					begin
						EN_1 <= 1;
						EN_2 <= 1;
						EN_3 <= EN_H0;
						EN_4 <= 1;
						SEG_A	<= DISPLAY_H0[0];
						SEG_B	<= DISPLAY_H0[1];
						SEG_C	<= DISPLAY_H0[2];
						SEG_D	<= DISPLAY_H0[3];
						SEG_E	<= DISPLAY_H0[4];
						SEG_F	<= DISPLAY_H0[5];
						SEG_G	<= DISPLAY_H0[6];
						count <= count + 1;
					end
				else if(count == 2'd3)
					begin
						EN_1 <= 1;
						EN_2 <= 1;
						EN_3 <= 1;
						EN_4 <= EN_H1;
						SEG_A	<= DISPLAY_H1[0];
						SEG_B	<= DISPLAY_H1[1];
						SEG_C	<= DISPLAY_H1[2];
						SEG_D	<= DISPLAY_H1[3];
						SEG_E	<= DISPLAY_H1[4];
						SEG_F	<= DISPLAY_H1[5];
						SEG_G	<= DISPLAY_H1[6];
						count <= 0;
					end					
			end		
		
	end
	
	endmodule