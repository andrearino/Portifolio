
module Mux_Prescaler (SEL, CLK_NORMAL, CLK_FAST, CLK_REL);


	input 				SEL;
	input 				CLK_NORMAL;
	input 				CLK_FAST;
	output reg			CLK_REL;
	

	always @(*) 
	begin
		if(~SEL)
			CLK_REL <= CLK_FAST;			 
		else if(SEL)
			CLK_REL = CLK_NORMAL;    
	end
endmodule