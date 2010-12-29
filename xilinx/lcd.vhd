-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity lcd is
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
end entity;

architecture behavioral of lcd is
    signal hcounter : std_logic_vector(9 downto 0);
    signal vcounter : std_logic_vector(8 downto 0);
    signal enablex  : std_logic;
    constant TH  : integer := 800; -- horizontal cycles
    constant THE : integer :=  48; -- enable setup time
    constant THP : integer :=  96; -- sync pulse width
    constant TEP : integer := 640; -- enable pulse width
    constant TV  : integer := 525; -- vertical cycles
    constant TVS : integer :=  34; -- data setup time
    constant TVP : integer :=   8; -- sync pulse width
    constant TVD : integer := 480; -- display data width
    constant propagation : integer := 2;
begin
    process(clock, reset)
    begin
        if (reset = '1') then
            hcounter <= conv_std_logic_vector(TEP+propagation, 10);
            vcounter <= conv_std_logic_vector(TVD, 9);
        elsif rising_edge(clock) then
            if (hcounter = TH-1) then
                hcounter <= (others => '0');
                if (vcounter = TV-1) then
                    vcounter <= (others => '0');
                else
                    vcounter <= vcounter + 1;
                end if;
            else
                hcounter <= hcounter + 1;
            end if;
	    end if;
    end process;

    enablex <= '1' when (propagation <= hcounter) and (hcounter < TEP + propagation)
                   else '0';
    enable <= enablex;
    hsync  <= '0' when (TH-THE-THP - propagation <= hcounter) and (hcounter < TH-THE - propagation)
                  else '1';
    vsync  <= '0' when (TV-TVS-TVP <= vcounter) and (vcounter < TV-TVS)
                  else '1';

    x <= hcounter when (hcounter < TEP)
                  else (others => '0');
    y <= vcounter when (vcounter < TVD)
                  else (others => '0');

    process(clock, reset)
    begin
        if (reset = '1') then
            ored <= (others => '0');
            ogreen <= (others => '0');
            oblue <= (others => '0');
        elsif rising_edge(clock) then
            if (propagation-1 <= hcounter) and (hcounter < TEP + propagation-1) then
                ored <= ired;
                ogreen <= igreen;
                oblue <= iblue;
            else
                ored <= (others => '0');
                ogreen <= (others => '0');
                oblue <= (others => '0');
            end if;
        end if;
    end process;
end architecture;