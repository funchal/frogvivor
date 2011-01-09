-- Giovanni Funchal <gafunchal@gmail.com>
-- Laurie Lugrin <marmottine@gmail.com>

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity deltasigmadac is
    generic(
        width  : integer := 8
    );
    port(
        reset  : in  std_logic;
        clock  : in  std_logic;
        dacin  : in  std_logic_vector(width-1 downto 0);
        dacout : out std_logic
    );
end entity;

architecture behavioral of deltasigmadac is
    signal deltaadder    : std_logic_vector(width+1 downto 0);
    signal sigmaadder    : std_logic_vector(width+1 downto 0);
    signal sigmalatch    : std_logic_vector(width+1 downto 0);
    signal deltafeedback : std_logic_vector(width+1 downto 0);
begin
    deltafeedback <= (sigmalatch(width+1), sigmalatch(width+1), others => '0');
    deltaadder    <= dacin + deltafeedback;
    sigmaadder    <= deltaadder + sigmalatch;

    process(clock, reset)
    begin
        if (reset = '1') then
            sigmalatch <= ('0', '1', others => '0');
            dacout <= '0';
        elsif rising_edge(clock) then
            sigmalatch <= sigmaadder;
            dacout <= sigmalatch(width+1);
	    end if;
    end process;
end architecture;