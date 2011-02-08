-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package frog_types is
    type frog_state is (frog, jump, splat);
	type frog_t is
        record
            hue      : integer range 0 to 5;
            x        : unsigned(9 downto 0);
            y        : unsigned(8 downto 0);
			-- line number where the frog is (from bottom)
			position : unsigned(5 downto 0);
            state    : frog_state;
			-- count when the frog should change state
			counter  : unsigned(3 downto 0);
			-- the user want to go up
			go       : std_logic;
        end record;
    type frogs_t is array(3 downto 0) of frog_t;

    type vehicle_kind is (none, car, truck);
    type vehicle_dir  is (lr, rl);
    type vehicle_t is
        record
            hue   : integer range 0 to 5;
            x     : unsigned(9 downto 0);
            y     : unsigned(8 downto 0);
            kind  : vehicle_kind;
            dir   : vehicle_dir;
        end record;
    type vehicles_t is array(1 downto 0) of vehicle_t;
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
        vehicles : in vehicles_t; 
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
    constant car_base_addr   : unsigned(14 downto 0) := to_unsigned(9216, 15);
    constant truck_base_addr : unsigned(14 downto 0) := to_unsigned(13824, 15);

    type frog_coef_t is array(3 downto 0) of integer;
    type vehicle_coef_t is array(1 downto 0) of integer;

    signal addrax       : unsigned(18 downto 0);
    signal addrbx       : unsigned(19 downto 0);
    signal frog_enable  : std_logic_vector(3 downto 0);
    signal vehicle_enable : std_logic_vector(1 downto 0);
    signal frog_enables : std_logic;
    signal vehicle_enables : std_logic;
    signal addra        : std_logic_vector(14 downto 0);
    signal douta        : std_logic_vector(7 downto 0);
    signal addrb        : std_logic_vector(14 downto 0);
    signal doutb        : std_logic_vector(7 downto 0);
    signal data         : std_logic_vector(7 downto 0);
    signal ux           : unsigned(9 downto 0);
    signal uy           : unsigned(8 downto 0);
    signal dx           : unsigned(9 downto 0);
    signal dx0          : unsigned(19 downto 0);
    signal dx1          : unsigned(19 downto 0);
    signal dy           : unsigned(8 downto 0);
    signal this_frog    : frog_t;
    signal this_vehicle : vehicle_t;
    signal frog_coef    : frog_coef_t;
    signal vehicle_coef : vehicle_coef_t;
    signal hue          : integer range 0 to 5;
begin

    ux <= unsigned(x);
    uy <= unsigned(y);

    frog_enable_generate :
    for i in 3 downto 0 generate
        frog_coef(i) <= 2 when frogs(i).state = jump else
                        1;
        frog_enable(i) <= '1' when ((ux >= frogs(i).x) and (ux < frogs(i).x + width) and
                                    (uy >= frogs(i).y) and (uy < frogs(i).y + frog_coef(i)*height)) else
                          '0';
    end generate;

    this_frog <= frogs(0) when frog_enable(0) = '1' else
                 frogs(1) when frog_enable(1) = '1' else
                 frogs(2) when frog_enable(2) = '1' else
                 frogs(3);

    addrax <= (ux - this_frog.x) + ((uy - this_frog.y) * width) when or_reduce(frog_enable) = '1' else
              to_unsigned(0, 19);

    addra  <= std_logic_vector(frog_base_addr  + addrax(14 downto 0)) when this_frog.state = frog  else
              std_logic_vector(splat_base_addr + addrax(14 downto 0)) when this_frog.state = splat else
              std_logic_vector(jump_base_addr  + addrax(14 downto 0));

    vehicle_enable_generate:
    for i in 1 downto 0 generate
        vehicle_coef(i) <= 3 when vehicles(i).kind = truck else
                           2;
        vehicle_enable(i) <= '1' when ((ux >= vehicles(i).x) and (ux < vehicles(i).x + vehicle_coef(i)*width) and
                                       (uy >= vehicles(i).y) and (uy < vehicles(i).y + height)) else
                             '0';
    end generate;

    this_vehicle <= vehicles(0) when vehicle_enable(0) = '1' else
                    vehicles(1);

    -- rotate/flip
    dy <= (uy - this_vehicle.y);
    dx0 <= 2*width;
    dx1 <= 3*width;
    dx <= (ux - this_vehicle.x) when this_vehicle.dir = rl else
          dx0(9 downto 0) - (ux - this_vehicle.x) when this_vehicle.kind = car else
          dx1(9 downto 0) - (ux - this_vehicle.x);

    addrbx <= (dy) + (dx * width) when or_reduce(vehicle_enable) = '1' else
              to_unsigned(0, 20);

    addrb  <= std_logic_vector(car_base_addr   + addrbx(14 downto 0)) when this_vehicle.kind = car else
              std_logic_vector(truck_base_addr + addrbx(14 downto 0));

    sprites_0 :
        entity work.sprites
        port map (
            rsta => reset,
            clka => clock,
            addra => addra,
            douta => douta,
            rstb => reset,
            clkb => clock,
            addrb => addrb,
            doutb => doutb
        );

    process(clock, reset)
    begin
        if (reset = '1') then
            frog_enables <= '0';
            vehicle_enables <= '0';
        elsif rising_edge(clock) then
            frog_enables <= or_reduce(frog_enable);
            vehicle_enables <= or_reduce(vehicle_enable);
	    end if;
    end process;

    enable <= '1' when ((vehicle_enables = '1') and (doutb /= "00000000")) or
                       ((frog_enables = '1') and (douta /= "00000000")) else
              '0';

    data <= doutb when (vehicle_enables = '1') and (doutb /= "00000000") else
            douta;

    hue <= this_vehicle.hue when ((vehicle_enables = '1') and (doutb /= "00000000")) else
           this_frog.hue;

    colorize_0 :
        entity work.colorize
        port map(
            hue   => hue,
            lum   => data,
            red   => red,
            green => green,
            blue  => blue
        );

end architecture;