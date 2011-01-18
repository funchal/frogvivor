-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.frog_types.all;

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
    signal frogs     : frogs_t;
    signal vehicles  : vehicles_t;
begin

    frogs(0).hue <= 4;
    frogs(1).hue <= 5;
    frogs(2).hue <= 1;
    frogs(3).hue <= 2;
    frogs(0).x <= to_unsigned(48*(4+2*0)-40,10);
    frogs(1).x <= to_unsigned(48*(4+2*1)-40,10);
    frogs(2).x <= to_unsigned(48*(4+2*2)-40,10);
    frogs(3).x <= to_unsigned(48*(4+2*3)-40,10);
    frogs(0).y <= to_unsigned(384,9);
    frogs(1).y <= to_unsigned(384,9);
    frogs(2).y <= to_unsigned(384,9);
    frogs(3).y <= to_unsigned(384,9);
    frogs(0).state <= frog;
    frogs(1).state <= jump;
    frogs(2).state <= splat;
    frogs(3).state <= jump;

    vehicles(0).hue <= 3;
    vehicles(1).hue <= 0;
    vehicles(0).x <= to_unsigned(373, 10);
    vehicles(0).y <= to_unsigned(370, 9);
    vehicles(1).x <= to_unsigned(256, 10);
    vehicles(1).y <= to_unsigned(64, 9);
    vehicles(0).kind <= car;
    vehicles(1).kind <= truck;
    vehicles(0).dir <= rl;
    vehicles(1).dir <= lr;
    
    draw_frog_0 :
        entity work.draw_frog
        port map(
            reset  => reset,
            clock  => pixel_clock,
            x      => x,
            y      => y,
            frogs  => frogs,
            vehicles => vehicles,
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