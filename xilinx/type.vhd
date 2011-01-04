-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

package types is

	subtype color_type is std_logic_vector(7 downto 0);
	type rgb_type is array (0 to 2) of color_type;
	type rgb_array_type is array (0 to 3) of rgb_type;
 
end package;
