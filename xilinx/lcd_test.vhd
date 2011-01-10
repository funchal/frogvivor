-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;
use work.txt_util.all;

entity lcd_test is
    generic(
        constant clock_period : time := 10 ns
    );
end entity;
 
architecture behavioral of lcd_test is
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

    signal reset   : std_logic;
    signal clock   : std_logic;
    signal ired    : std_logic_vector(5 downto 0);
    signal igreen  : std_logic_vector(5 downto 0);
    signal iblue   : std_logic_vector(5 downto 0);
    signal ored    : std_logic_vector(5 downto 0);
    signal ogreen  : std_logic_vector(5 downto 0);
    signal oblue   : std_logic_vector(5 downto 0);
    signal enable  : std_logic;
    signal hsync   : std_logic;
    signal vsync   : std_logic;
    signal x       : std_logic_vector(9 downto 0);
    signal y       : std_logic_vector(8 downto 0);

begin 
 
    dut: lcd
    port map(
        reset => reset,
        clock => clock,
        ired => ired,
        igreen => igreen,
        iblue => iblue,
        ored => ored,
        ogreen => ogreen,
        oblue => oblue,
        enable => enable,
        hsync => hsync,
        vsync => vsync,
        x => x,
        y => y
    );

    clock_process : process
    begin
        clock <= '0';
        wait for clock_period/2;
        clock <= '1';
        wait for clock_period/2;
    end process;

    hsl2rgb_0 : entity work.hsl2rgb port map(
        hue    => 3,
        lum    => y(7 downto 0),
        ored   => ired,
        ogreen => igreen,
        oblue  => iblue
    );

    --ired <= (others => x(6));
    --igreen <= (others => x(7));
    --iblue <= y(8 downto 3);

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
                    write(f, character'val(to_integer(unsigned(oblue  & "00"))));
                    write(f, character'val(to_integer(unsigned(ogreen & "00"))));
                    write(f, character'val(to_integer(unsigned(ored   & "00"))));
                end loop;
            end loop;

            file_close(f);

            report "frame " & str(c) & " end";

        end loop;
        report "end of testbench";
        wait;
    end process;
end;