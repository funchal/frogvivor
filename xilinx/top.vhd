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
    component dcm32to28 is
        port(
            RST_IN          : in  std_logic;
            CLKIN_IN        : in  std_logic;          
            CLKFX_OUT       : out std_logic;
            CLKIN_IBUFG_OUT : out std_logic;
            CLK0_OUT        : out std_logic
        );
	end component;

    signal reset       : std_logic;
    signal pixel_clock : std_logic;
begin

    dcm32to28_0 :
        dcm32to28
        port map(
            RST_IN => reset,
            CLKIN_IN => clock,
            CLKFX_OUT => pixel_clock,
            CLKIN_IBUFG_OUT => open,
            CLK0_OUT => open
        );

    frogvivor_0 :
        entity work.frogvivor
        port map(
            reset       => reset,
            pixel_clock => pixel_clock,
            red(0)      => wingBL(4),
            red(1)      => wingAH(4),
            red(2)      => wingBL(5),
            red(3)      => wingAH(3),
            red(4)      => wingBL(6),
            red(5)      => wingAH(2),
            green(0)    => wingAH(1),
            green(1)    => wingBL(7),
            green(2)    => wingAH(0),
            green(3)    => wingBH(0),
            green(4)    => wingAL(7),
            green(5)    => wingBH(1),
            blue(0)     => wingBH(2),
            blue(1)     => wingAL(6),
            blue(2)     => wingBH(3),
            blue(3)     => wingAL(5),
            blue(4)     => wingBH(4),
            blue(5)     => wingAL(4),
            enable      => wingAL(3),
            hsync       => wingAH(5),
            vsync       => wingBL(3),
            hflip       => wingBH(5),
            vflip       => wingAL(2),
            dacout      => wingAH(6)
        );

    reset <= '0';

    wingBL(2) <= pixel_clock;

    -- tip/left/mono channel
    -- audiotip <= wingAH(6);

    -- ring/right channel
    -- audioring <= wingAH(7);

    -- connected to tip when not plugged in, otherwise open
    -- tipswitch <= wingBL(0);
    
    -- connected to ring when mono, otherwise open
    -- ringswitch <= wingBL(1);
    
end architecture;