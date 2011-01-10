-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity hsl2rgb is
    port(
        hue    : in  integer range 0 to 5;
        lum    : in  std_logic_vector(7 downto 0);
        ored   : out std_logic_vector(5 downto 0);
        ogreen : out std_logic_vector(5 downto 0);
        oblue  : out std_logic_vector(5 downto 0)
    );
end entity;

architecture behavioral of hsl2rgb is
    signal p : std_logic_vector(15 downto 0);
    signal q : std_logic_vector(7 downto 0);
begin

    p <= std_logic_vector((unsigned(lum) * unsigned(lum)));
    sqrt_0: entity work.sqrt16 port map(
        P(15 downto 8) => lum,
        P(7 downto 0) => "00000000",
        U => q
    );

    ored   <= p(15 downto 10) when hue = 2 or hue = 3 or hue = 4 else q(7 downto 2);
    ogreen <= p(15 downto 10) when hue = 4 or hue = 5 or hue = 0 else q(7 downto 2);
    oblue  <= p(15 downto 10) when hue = 0 or hue = 1 or hue = 2 else q(7 downto 2);

end architecture;