
# PlanAhead Launch Script for Post PAR Floorplanning, created by Project Navigator

create_project -name project -dir "/home/funchal/frogvivor/xilinx/planAhead_run_2" -part xc3s250evq100-4
set srcset [get_property srcset [current_run -impl]]
set_property design_mode GateLvl $srcset
set_property edif_top_file "/home/funchal/frogvivor/xilinx/top.ngc" [ get_property srcset [ current_run ] ]
add_files -norecurse { {/home/funchal/frogvivor/xilinx} }
set_param project.paUcfFile  "/home/funchal/frogvivor/xilinx/BPC3003_2.03+_alternative.ucf"
add_files "BPC3003_2.03+_alternative.ucf" -fileset [get_property constrset [current_run]]
add_files "demo.ucf" -fileset [get_property constrset [current_run]]
open_netlist_design
read_xdl -file "/home/funchal/frogvivor/xilinx/top.xdl"
if {[catch {read_twx -name results_1 -file "/home/funchal/frogvivor/xilinx/top.twx"} eInfo]} {
   puts "WARNING: there was a problem importing \"/home/funchal/frogvivor/xilinx/top.twx\": $eInfo"
}
