-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity pow2 is
    port(
        x : in  std_logic_vector(7 downto 0);
        y : out std_logic_vector(15 downto 0)
    );
end entity;

architecture behavioral of pow2 is
    signal w : std_logic_vector(15 downto 0);
begin
--    21 slices, 38 lut, 13.374ns
    w(15 downto 8) <= (others => '0');
    w(7 downto 0) <= x;
    y <= std_logic_vector((unsigned(w) * unsigned(w)));
--    4 slices, 8 lut
--    w(15 downto 8) <= (others => '0');
--    w(7 downto 0) <= x;
--    y <= std_logic_vector((unsigned(w) sll 7) srl 8) when w(7) = '1' else
--         std_logic_vector((unsigned(w) sll 6) srl 8) when w(6) = '1' else
--         std_logic_vector((unsigned(w) sll 5) srl 8) when w(5) = '1' else
--         std_logic_vector((unsigned(w) sll 4) srl 8) when w(4) = '1' else
--         std_logic_vector((unsigned(w) sll 3) srl 8) when w(3) = '1' else
--         std_logic_vector((unsigned(w) sll 2) srl 8) when w(2) = '1' else
--         std_logic_vector((unsigned(w) sll 1) srl 8) when w(1) = '1' else
--         std_logic_vector((unsigned(w) sll 0) srl 8);
end architecture;