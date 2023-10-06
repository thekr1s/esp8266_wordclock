import os
import glob
import sys

# Script varibles
Threshold_Height            = 4 #Set the threshold for when a rapid move should happen
override_feed_rate          = 10001 #Set rapid move feedrate
file_extention              = "ngc"

def fix_gcode_file(SelectedFiles):
    feedrate_overruled = False
    prv_feedrate = None
    new_file_path = SelectedFiles.split("." + file_extention)[0]+ "_FIXED." + file_extention
    with open(new_file_path, "w") as f_out:
        with open(SelectedFiles) as f:
            for line in f:
                #don't change gcode if its in comment
                if not line.startswith("(") and ("Z" in line.upper() or "F" in line.upper()):
                    overruled_cmds = ""
                    for cmd in line.split(): # Split the line in separted Gcode commands
                        if "F" in cmd.upper():
                            prv_feedrate = float(cmd.upper().replace("F","")) # Get the value from F command
                        if "Z" in cmd.upper():
                            Current_Height = float(cmd.upper().replace("Z","")) # Get the value from Z command
                            if Current_Height > Threshold_Height:
                                if feedrate_overruled == False:
                                    # Head is above threshold, increase fead rate
                                    if prv_feedrate:
                                        cmd = "F%d %s" % (override_feed_rate, cmd)
                                feedrate_overruled = True
                            else:
                                if feedrate_overruled == True:
                                    # Head is below threshold, restore fead rate if present
                                    if prv_feedrate:
                                        cmd = "F%d %s" % (prv_feedrate, cmd)
                                feedrate_overruled = False
                        overruled_cmds += cmd + " "
                    f_out.write(overruled_cmds.strip() + "\n")
                else:
                    f_out.writelines(line)
    f_out.close()

if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1]:
        search_folder = sys.argv[1]
    else:
        search_folder = os.getcwd()

    for filepath in glob.glob(os.path.join(search_folder, "*." + file_extention)):
        filename = os.path.basename(filepath)
        if "_FIXED." + file_extention in filename:
            continue
        
        #result = input('Fix %s?, yes or no\n' % filename)
        result = "YES"
        if result.upper() == "YES" or result.upper() == "Y":
            print("Adding %s for fixing reapit movement" % filename)
            fix_gcode_file(filepath)