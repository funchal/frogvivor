-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;
use work.txt_util.all;

entity test is
    generic(
        constant clock_period : time := 10 ns
    );
end entity;
 
architecture testbench of test is
    signal reset    : std_logic;
    signal clock    : std_logic;
    signal red      : std_logic_vector(5 downto 0);
    signal green    : std_logic_vector(5 downto 0);
    signal blue     : std_logic_vector(5 downto 0);
    signal enable   : std_logic;
    signal hsync    : std_logic;
    signal vsync    : std_logic;
    signal hflip    : std_logic;
    signal vflip    : std_logic;
begin 

    clock_process : process
    begin
        clock <= '0';
        wait for clock_period/2;
        clock <= '1';
        wait for clock_period/2;
    end process;

    frogvivor_0 :
        entity work.frogvivor
        port map(
            reset       => reset,
            pixel_clock => clock,
            red         => red,
            green       => green,
            blue        => blue,
            enable      => enable,
            hsync       => hsync,
            vsync       => vsync,
            hflip       => hflip,
            vflip       => vflip,
            dacout      => open
        );

    process
        type char_file_t is file of character;
        file f : char_file_t;
        constant header : std_logic_vector(54*8-1 downto 0) :=
            x"424D"     & -- "BM" magic
            x"36100E00" & -- file size 640*480*3 + 54 = 921654 in little-endian
            x"00000000" & -- reserved
            x"36000000" & -- offset to data 56
            x"28000000" & -- header size 40
            x"80020000" & -- width 640
            x"E0010000" & -- height 480
            x"0100"     & -- color planes 1
            x"1800"     & -- bits per pixel 24
            x"00000000" & -- compression none
            x"00100E00" & -- image size 640*480*3 = 921600
            x"130b0000" & -- horizontal resolution 72dpi = 2835 pixel per meter
            x"130b0000" & -- vertical resolution
            x"00000000" & -- color pallete
            x"00000000";  -- ignored
    begin
        -- reset
        reset <= '1';
        wait for clock_period*10;
        reset <= '0';

        for c in 0 to 1 loop
            
            file_open(f, "output-" & str(c) & ".bmp", WRITE_MODE);

            -- write bmp header
            for i in 53 downto 0 loop
                write(f, character'val(to_integer(unsigned(header(8*(i+1)-1 downto 8*i)))));
            end loop;

            -- write data
            wait until rising_edge(vsync);
            for i in 0 to 479 loop
                wait until rising_edge(enable);
                for j in 0 to 639 loop
                    wait until falling_edge(clock);
                    write(f, character'val(to_integer(unsigned(blue  & "00"))));
                    write(f, character'val(to_integer(unsigned(green & "00"))));
                    write(f, character'val(to_integer(unsigned(red   & "00"))));
                end loop;
            end loop;

            file_close(f);

            report "frame " & str(c) & " end";

        end loop;
        report "end of testbench";
        wait;
    end process;
end architecture;