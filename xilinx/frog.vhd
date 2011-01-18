-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package frog_types is
    type frog_state is (frog, jump, splat);
    type frog_t is
        record
            hue   : integer range 0 to 5;
            x     : unsigned(9 downto 0);
            y     : unsigned(8 downto 0);
            state : frog_state;
        end record;
    type frogs_t is array(3 downto 0) of frog_t;
end package;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.frog_types.all;
use work.reduce_pack.all;

entity draw_frog is
    port(
        reset  : in  std_logic;
        clock  : in  std_logic;
        x      : in  std_logic_vector(9 downto 0);
        y      : in  std_logic_vector(8 downto 0);
        frogs  : in  frogs_t;
        red    : out std_logic_vector(5 downto 0);
        green  : out std_logic_vector(5 downto 0);
        blue   : out std_logic_vector(5 downto 0);
        enable : out std_logic
    );
end entity;
 
architecture behavioral of draw_frog is
    constant width  : unsigned(9 downto 0) := to_unsigned(48, 10);
    constant height : unsigned(8 downto 0) := to_unsigned(48, 9);
    constant frog_base_addr  : unsigned(14 downto 0) := to_unsigned(0, 15);
    constant jump_base_addr  : unsigned(14 downto 0) := to_unsigned(2304, 15);
    constant splat_base_addr : unsigned(14 downto 0) := to_unsigned(6912, 15);

    signal addrax      : unsigned(18 downto 0);
    signal frog_enable : std_logic_vector(3 downto 0);
    signal enablex     : std_logic;
    signal addra       : std_logic_vector(14 downto 0);
    signal douta       : std_logic_vector(7 downto 0);
    signal addrb       : std_logic_vector(14 downto 0);
    signal doutb       : std_logic_vector(7 downto 0);
    signal ux          : unsigned(9 downto 0);
    signal uy          : unsigned(8 downto 0);
    signal this_frog   : frog_t;
begin

    ux <= unsigned(x);
    uy <= unsigned(y);

    frog_enable_generate :
    for i in 3 downto 0 generate
        frog_enable(i) <= '1' when ((ux >= frogs(i).x) and (ux < frogs(i).x + width) and
                                    (uy >= frogs(i).y) and (uy < frogs(i).y + height)) else
                          '0';
    end generate;

    this_frog <= frogs(0) when frog_enable(0) = '1' else
                 frogs(1) when frog_enable(1) = '1' else
                 frogs(2) when frog_enable(2) = '1' else
                 frogs(3);

    addrax <= (ux - this_frog.x) + ((uy - this_frog.y) * width);

    addra  <= std_logic_vector(frog_base_addr + addrax(14 downto 0))  when this_frog.state = frog  else
              std_logic_vector(splat_base_addr + addrax(14 downto 0)) when this_frog.state = splat else
              std_logic_vector(jump_base_addr + addrax(14 downto 0));

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
            hue   => this_frog.hue,
            lum   => douta,
            red   => red,
            green => green,
            blue  => blue
        );

    addrb <= (others => '0');

    process(clock, reset)
    begin
        if (reset = '1') then
            enablex <= '0';
        elsif rising_edge(clock) then
            enablex <= or_reduce(frog_enable);
	    end if;
    end process;

    enable <= '1' when (enablex = '1') and (douta /= "00000000") else '0';

end architecture;