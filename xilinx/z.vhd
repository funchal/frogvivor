-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

use work.types.all;

entity z is
	port (
		-- input from background (roads and grass)
		input : in rgb_array_type;
		
		-- output to LCD
		output : out rgb_type
		);
end z;

architecture Behavioral of z is

	-- pink means transparent
	constant transparent : rgb_type := ((others => (others => '0')));

begin

	output <= input(0) when input(0) /= transparent else
			  input(1) when input(1) /= transparent else
			  input(2) when input(2) /= transparent else
			  input(3);

end Behavioral;

