module Counter_5 (Q, CLK_IN, CLK_OUT, RST, BCD_A, BCD_B, BCD_C, BCD_D);


	input 				CLK_IN;
	input 				RST;
	output reg			CLK_OUT;
	output reg	[3:0]	Q;
	output				BCD_A;
	output				BCD_B;
	output				BCD_C;
	output				BCD_D;
	

	reg 			[3:0]	count;
	


	always @ (posedge CLK_IN or posedge RST)
	begin
		if (RST)
			count <= 0;

		else if (CLK_OUT == 1)
			begin
				CLK_OUT <= 0;
				count <= count + 1;
			end
		else if (count == 4'd5)
			begin
				CLK_OUT <= 1;
				count <= 0;
			end
		else
			count <= count + 1;
	end
	
	assign BCD_A = count[0];
	assign BCD_B = count[1];
	assign BCD_C = count[2];	
	assign BCD_D = count[3];
	
endmodule
