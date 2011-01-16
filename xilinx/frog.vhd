-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity frog is
    port(
        reset  : in  std_logic;
        clock  : in  std_logic;
        addr   : in  std_logic_vector(13 downto 0);
        data   : out std_logic_vector(7 downto 0)
    );
end entity;
 
architecture behavioral of frog is
    component sprites
        port(
            clka  : in  std_logic;
            rsta  : in  std_logic;
            addra : in  std_logic_vector(13 downto 0);
            douta : out std_logic_vector(7 downto 0);
            clkb  : in  std_logic;
            rstb  : in  std_logic;
            addrb : in  std_logic_vector(13 downto 0);
            doutb : out std_logic_vector(7 downto 0)
        );
    end component;
    signal addrb : std_logic_vector(13 downto 0);
    signal doutb : std_logic_vector(7 downto 0);
begin

    addrb <= (others => '0');
    sprites_0 : sprites
        port map (
            clka => clock,
            rsta => reset,
            addra => addr,
            douta => data,
            clkb => clock,
            rstb => reset,
            addrb => addrb,
            doutb => doutb
        );
            
end architecture;