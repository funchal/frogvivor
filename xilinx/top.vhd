-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity top is
    port(
        wingAH : inout std_logic_vector(7 downto 0);
        wingAL : inout std_logic_vector(7 downto 0);
        wingBH : inout std_logic_vector(7 downto 0);
        wingBL : inout std_logic_vector(7 downto 0);
        wingCH : inout std_logic_vector(7 downto 0);
        wingCL : inout std_logic_vector(7 downto 0);
        rx     : in    std_logic;
        tx     : inout std_logic;
        clock  : in    std_logic
    );
end entity;

architecture behavioral of top is
    component lcd is
        port(
            reset  : in  std_logic;
            clock  : in  std_logic;
            ired   : in  std_logic_vector(5 downto 0);
            igreen : in  std_logic_vector(5 downto 0);
            iblue  : in  std_logic_vector(5 downto 0);
            ored   : out std_logic_vector(5 downto 0);
            ogreen : out std_logic_vector(5 downto 0);
            oblue  : out std_logic_vector(5 downto 0);
            enable : out std_logic;
            hsync  : out std_logic;
            vsync  : out std_logic;
            x      : out std_logic_vector(9 downto 0);
            y      : out std_logic_vector(8 downto 0)
        );
    end component;

	component dcm32to28 is
        port(
            RST_IN          : IN  std_logic;
            CLKIN_IN        : IN  std_logic;          
            CLKFX_OUT       : OUT std_logic;
            CLKIN_IBUFG_OUT : OUT std_logic;
            CLK0_OUT        : OUT std_logic
        );
	end component;

    component deltasigmadac is
        generic(
            width  : integer := 8
        );
        port(
            reset  : in  std_logic;
            clock  : in  std_logic;
            dacin  : in  std_logic_vector(width-1 downto 0);
            dacout : out std_logic
        );
    end component;

    signal reset       : std_logic;
    signal pixel_clock : std_logic;
    signal audio_clock : std_logic;
    signal x           : std_logic_vector(9 downto 0);
    signal y           : std_logic_vector(8 downto 0);
    signal red         : std_logic_vector(5 downto 0);
    signal green       : std_logic_vector(5 downto 0);
    signal blue        : std_logic_vector(5 downto 0);
    signal hflip       : std_logic;
    signal vflip       : std_logic;
    signal audioin     : std_logic_vector(7 downto 0);
    signal mult        : std_logic_vector(1 downto 0);
begin

    dcm32to28_0 : dcm32to28
        port map(
            RST_IN => reset,
            CLKIN_IN => clock,
            CLKFX_OUT => pixel_clock,
            CLKIN_IBUFG_OUT => open,
            CLK0_OUT => open
        );

    lcd_0 : lcd
        port map(
            reset     => reset,
            clock     => pixel_clock,
            ired      => red,
            igreen    => green,
            iblue     => blue,
            ored(0)   => wingBL(4),
            ored(1)   => wingAH(4),
            ored(2)   => wingBL(5),
            ored(3)   => wingAH(3),
            ored(4)   => wingBL(6),
            ored(5)   => wingAH(2),
            ogreen(0) => wingAH(1),
            ogreen(1) => wingBL(7),
            ogreen(2) => wingAH(0),
            ogreen(3) => wingBH(0),
            ogreen(4) => wingAL(7),
            ogreen(5) => wingBH(1),
            oblue(0)  => wingBH(2),
            oblue(1)  => wingAL(6),
            oblue(2)  => wingBH(3),
            oblue(3)  => wingAL(5),
            oblue(4)  => wingBH(4),
            oblue(5)  => wingAL(4),
            enable    => wingAL(3),
            hsync     => wingAH(5),
            vsync     => wingBL(3),
            x         => x,
            y         => y
        );

    red <= x(5 downto 0);
    green <= (others => x(5) and y(3));
    blue <= y(8 downto 3);
    
    reset <= '0';

    hflip <= '1';
    vflip <= '1';

    wingBH(5) <= hflip;
    wingAL(2) <= vflip;

    wingBL(2) <= pixel_clock;

    process(pixel_clock, reset)
    begin
        if (reset = '1') then
            audioin <= (others => '0');
            mult <= (others => '0');
        elsif rising_edge(pixel_clock) then
            audioin <= audioin * mult;
            if(audioin > b"11111100") then
                mult <= mult + 1;
            end if;
        end if;
    end process;

    deltasigmadac_0 : deltasigmadac
        port map(
            reset  => reset,
            clock  => pixel_clock,
            dacin  => audioin,
            dacout => wingAH(6)
        );

    -- tip/left/mono channel
    -- audiotip <= wingAH(6);

    -- ring/right channel
    -- audioring <= wingAH(7);

    -- connected to tip when not plugged in, otherwise open
    -- tipswitch <= wingBL(0);
    
    -- connected to ring when mono, otherwise open
    -- ringswitch <= wingBL(1);
    
end architecture;