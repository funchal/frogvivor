-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity demo is
    port(
        x : in  std_logic_vector(7 downto 0);
        y : out std_logic_vector(7 downto 0)
    );
end entity;

architecture behavioral of demo is
begin
    y <= std_logic_vector((unsigned(x) srl 4) sll 4) when x(7) = '1' else
         std_logic_vector((unsigned(x) srl 3) sll 4) when x(6) = '1' else
         std_logic_vector((unsigned(x) srl 3) sll 4) when x(5) = '1' else
         std_logic_vector((unsigned(x) srl 2) sll 4) when x(4) = '1' else
         std_logic_vector((unsigned(x) srl 2) sll 4) when x(3) = '1' else
         std_logic_vector((unsigned(x) srl 1) sll 4) when x(2) = '1' else
         std_logic_vector((unsigned(x) srl 1) sll 4) when x(1) = '1' else
         std_logic_vector((unsigned(x) srl 0) sll 4);
end architecture;