
module Reset_24 (CLK, BCD_H0, BCD_H1, BCD_M0, BCD_M1, BCD_S0, BCD_S1, RST);


	input 				CLK;
	input	[3:0]			BCD_H0;
	input	[3:0]			BCD_H1;
	input	[3:0]			BCD_M0;
	input	[3:0]			BCD_M1;
	input	[3:0]			BCD_S0;
	input	[3:0]			BCD_S1;
	output reg			RST;
	

	reg	[3:0]			H0;
	reg	[3:0]			H1;
	reg	[3:0]			M0;
	reg	[3:0]			M1;
	reg	[3:0]			S0;
	reg	[3:0]			S1;
	

	always @ (posedge CLK)
	begin
		H0 <= BCD_H0;
		H1 <= BCD_H1;
		M0 <= BCD_M0;
		M1 <= BCD_M1;
		S0 <= BCD_S0;
		S1 <= BCD_S1;
		
		RST <= 0;
		
		if (H1 == 4'd2)
			if (H0 == 4'd3)
				if (M1 == 4'd5)
					if (M0 == 4'd9)
						if (S1 == 4'd5)
							if (S0 == 4'd9)
								begin
									RST <= 1;
								end
		else
			RST <= 0;						
	end
	
endmodule
