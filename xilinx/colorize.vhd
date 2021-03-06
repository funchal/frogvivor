-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity colorize is
    port(
        hue   : in  integer range 0 to 5;
        lum   : in  std_logic_vector(7 downto 0);
        red   : out std_logic_vector(5 downto 0);
        green : out std_logic_vector(5 downto 0);
        blue  : out std_logic_vector(5 downto 0)
    );
end entity;

architecture behavioral of colorize is
    signal p : std_logic_vector(15 downto 0);
    signal q : std_logic_vector(15 downto 0);
begin

    p <= std_logic_vector((unsigned(lum) * unsigned(lum)));
    q <= std_logic_vector((unsigned(lum) sll 1) - (unsigned(p) srl 8) - 1)
         when lum /= "00000000" else (others => '0');

    red   <= p(15 downto 10) when hue = 2 or hue = 3 or hue = 4 else q(7 downto 2);
    green <= p(15 downto 10) when hue = 4 or hue = 5 or hue = 0 else q(7 downto 2);
    blue  <= p(15 downto 10) when hue = 0 or hue = 1 or hue = 2 else q(7 downto 2);

end architecture;