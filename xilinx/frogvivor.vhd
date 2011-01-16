-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity frogvivor is
    port(
        reset        : in  std_logic;
        pixel_clock  : in  std_logic;
        dacout       : out std_logic;
        red          : out std_logic_vector(5 downto 0);
        green        : out std_logic_vector(5 downto 0);
        blue         : out std_logic_vector(5 downto 0);
        enable       : out std_logic;
        hsync        : out std_logic;
        vsync        : out std_logic;
        vflip        : out std_logic;
        hflip        : out std_logic
    );
end entity;

architecture behavioral of frogvivor is
    signal enable_0  : std_logic;
    signal red_0     : std_logic_vector(5 downto 0);
    signal green_0   : std_logic_vector(5 downto 0);
    signal blue_0    : std_logic_vector(5 downto 0);
    signal lcd_red   : std_logic_vector(5 downto 0);
    signal lcd_green : std_logic_vector(5 downto 0);
    signal lcd_blue  : std_logic_vector(5 downto 0);
    signal audioin   : std_logic_vector(7 downto 0);
    signal mult      : std_logic_vector(1 downto 0);
    signal x         : std_logic_vector(9 downto 0);
    signal y         : std_logic_vector(8 downto 0);
begin

    frog_0 :
        entity work.frog
        port map(
            reset  => reset,
            clock  => pixel_clock,
            hue    => 3,
            x      => x,
            y      => y,
            red    => red_0,
            green  => green_0,
            blue   => blue_0,
            enable => enable_0
        );

    lcd_red   <= red_0   when enable_0 = '1' else "010000";
    lcd_green <= green_0 when enable_0 = '1' else "100000";
    lcd_blue  <= blue_0  when enable_0 = '1' else "001000";

    lcd_0:
        entity work.lcd
        port map(
            reset => reset,
            clock => pixel_clock,
            ired => lcd_red,
            igreen => lcd_green,
            iblue => lcd_blue,
            ored => red,
            ogreen => green,
            oblue => blue,
            enable => enable,
            hsync => hsync,
            vsync => vsync,
            x => x,
            y => y
        );

    hflip <= '1';
    vflip <= '1';

    audioin <= (others => '0');

    deltasigmadac_0 :
        entity work.deltasigmadac
        port map(
            reset  => reset,
            clock  => pixel_clock,
            dacin  => audioin,
            dacout => dacout
        );

end architecture;