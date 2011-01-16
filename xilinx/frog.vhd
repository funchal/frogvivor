-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity frog is
    port(
        reset  : in  std_logic;
        clock  : in  std_logic;
        hue    : in  integer range 0 to 5;
        x      : in  std_logic_vector(9 downto 0);
        y      : in  std_logic_vector(8 downto 0);
        red    : out std_logic_vector(5 downto 0);
        green  : out std_logic_vector(5 downto 0);
        blue   : out std_logic_vector(5 downto 0);
        enable : out std_logic
    );
end entity;
 
architecture behavioral of frog is
    constant width  : integer := 48;
    constant height : integer := 48;
    constant posx   : integer := 64;
    constant posy   : integer := 64;

    signal enablex  : std_logic;
    signal addra    : std_logic_vector(13 downto 0);
    signal douta    : std_logic_vector(7 downto 0);
    signal addrb    : std_logic_vector(13 downto 0);
    signal doutb    : std_logic_vector(7 downto 0);
    signal ux       : unsigned(9 downto 0);
    signal uy       : unsigned(8 downto 0);
begin

    ux <= unsigned(x);
    uy <= unsigned(y);

    enablex <= '1' when ((ux >= posx) and (ux < posx + width) and
                         (uy >= posy) and (uy < posy + height)) else
               '0';

    addra <= std_logic_vector(((ux - posx) + (uy - posy) * width) sll 4) when enablex = '1' else
             (others => '0');

    addrb <= (others => '0');

    sprites_0 :
        entity work.sprites
        port map (
            clka => clock,
            rsta => reset,
            addra => addra,
            douta => douta,
            clkb => clock,
            rstb => reset,
            addrb => addrb,
            doutb => doutb
        );

    colorize_0 :
        entity work.colorize
        port map(
            hue    => hue,
            lum    => douta,
            ored   => red,
            ogreen => green,
            oblue  => blue
        );

    enable <= '1' when enablex = '1' and (douta /= "00000000") else '0';

end architecture;