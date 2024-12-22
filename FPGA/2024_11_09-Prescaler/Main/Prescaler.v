module Prescaler (clk_in, rst, clk_1kHz, clk_100Hz, clk_10Hz, clk_1Hz, clk_c);

parameter main_clock = 50_000_000;
parameter custom_clock = 300;


reg [31:0] count_1kHz  	= 0;
reg [31:0] count_100Hz  = 0;
reg [31:0] count_10Hz  	= 0;
reg [31:0] count_1Hz		= 0;
reg [31:0] count_c		= 0;

input clk_in;					
input rst;						
output reg clk_1kHz;       
output reg clk_100Hz;      
output reg clk_10Hz;       
output reg clk_1Hz;        
output reg clk_c;          

always @(posedge clk_in)
	begin

	if (~rst)
			begin
				count_1kHz <= 0;
				count_100Hz <= 0;
				count_10Hz <= 0;
				count_1Hz <= 0;
				count_c <= 0;
			end
			
		else if (count_1kHz == (main_clock/1000/2)-1)
			begin
				clk_1kHz = ~clk_1kHz;
				count_1kHz <= 0;
			end

		else if (count_100Hz == (main_clock/100/2)-1)
			begin
				clk_100Hz = ~clk_100Hz;
				count_100Hz <= 0;
			end

		else if (count_10Hz == (main_clock/10/2)-1)
			begin
				clk_10Hz = ~clk_10Hz;
				count_10Hz <= 0;
			end

		else if (count_1Hz == (main_clock/2)-1)
			begin
				clk_1Hz = ~clk_1Hz;
				count_1Hz <= 0;
			end

		else if (count_c == (main_clock/custom_clock/2)-1)
			begin
				clk_c = ~clk_c;
				count_c <= 0;
			end
		
		else
			begin
				count_1kHz <= count_1kHz + 1;
				count_100Hz <= count_100Hz + 1;
				count_10Hz <= count_10Hz + 1;
				count_1Hz <= count_1Hz + 1;
				count_c <= count_c + 1;
			end
	end	
	
	
endmodule