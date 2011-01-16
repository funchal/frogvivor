-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity frog is
    port(
        reset  : in  std_logic;
        clock  : in  std_logic;
        x      : in  std_logic_vector(9 downto 0);
        y      : in  std_logic_vector(8 downto 0);
        hue_0  : in  integer range 0 to 5;
        hue_1  : in  integer range 0 to 5;
        hue_2  : in  integer range 0 to 5;
        hue_3  : in  integer range 0 to 5;
        posx_0 : in  unsigned(9 downto 0);
        posx_1 : in  unsigned(9 downto 0);
        posx_2 : in  unsigned(9 downto 0);
        posx_3 : in  unsigned(9 downto 0);
        posy_0 : in  unsigned(8 downto 0);
        posy_1 : in  unsigned(8 downto 0);
        posy_2 : in  unsigned(8 downto 0);
        posy_3 : in  unsigned(8 downto 0);
        red    : out std_logic_vector(5 downto 0);
        green  : out std_logic_vector(5 downto 0);
        blue   : out std_logic_vector(5 downto 0);
        enable : out std_logic
    );
end entity;
 
architecture behavioral of frog is
    constant width  : unsigned(7 downto 0) := to_unsigned(48, 8);
    constant height : unsigned(7 downto 0) := to_unsigned(48, 8);

    signal posx     : unsigned(9 downto 0);
    signal posy     : unsigned(8 downto 0);
    signal huex     : integer range 0 to 5;
    signal addrax   : unsigned(16 downto 0);
    signal enable_0 : std_logic;
    signal enable_1 : std_logic;
    signal enable_2 : std_logic;
    signal enable_3 : std_logic;
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

    enable_0 <= '1' when ((ux >= posx_0) and (ux < posx_0 + width) and
                          (uy >= posy_0) and (uy < posy_0 + height)) else
                '0';

    enable_1 <= '1' when ((ux >= posx_1) and (ux < posx_1 + width) and
                          (uy >= posy_1) and (uy < posy_1 + height)) else
                '0';

    enable_2 <= '1' when ((ux >= posx_2) and (ux < posx_2 + width) and
                          (uy >= posy_2) and (uy < posy_2 + height)) else
                '0';

    enable_3 <= '1' when ((ux >= posx_3) and (ux < posx_3 + width) and
                          (uy >= posy_3) and (uy < posy_3 + height)) else
                '0';

    enablex <= enable_0 or enable_1 or enable_2 or enable_3;

    posx <= posx_0 when enable_0 = '1' else
            posx_1 when enable_1 = '1' else
            posx_2 when enable_2 = '1' else
            posx_3;

    posy <= posy_0 when enable_0 = '1' else
            posy_1 when enable_1 = '1' else
            posy_2 when enable_2 = '1' else
            posy_3;

    huex <= hue_0 when enable_0 = '1' else
            hue_1 when enable_1 = '1' else
            hue_2 when enable_2 = '1' else
            hue_3;

    addrax <= (ux - posx) + ((uy - posy) * width);

    addra  <= std_logic_vector(addrax(13 downto 0)) when enablex = '1' else
              (others => '0');

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
            hue    => huex,
            lum    => douta,
            ored   => red,
            ogreen => green,
            oblue  => blue
        );

    addrb <= (others => '0');

    enable <= '1' when enablex = '1' and (douta /= "00000000") else '0';

end architecture;