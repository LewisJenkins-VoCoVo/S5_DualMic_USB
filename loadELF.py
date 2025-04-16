import os
import shutil

# Get the current working directory
current_directory = os.getcwd()
print("Current Working Directory:", current_directory)

# Define the working directory and the file paths
workingDir = '/Users/lewis.jenkins/Downloads/1-test/microphone-library-for-pico-lib-changes-3-mics'
target = os.path.join(workingDir, 'build', 'pico_microphone.elf')
source = os.path.join(workingDir, 'build', 'examples', 'usb_microphone', 'usb_microphone.elf')

# Print the source and target for confirmation
print("Source file:", source)
print("Target file:", target)

# Check if the source file exists
if not os.path.exists(source):
    print("Error: The source file does not exist.")
else:
    # Optionally, ensure that the target directory exists
    target_dir = os.path.dirname(target)
    os.makedirs(target_dir, exist_ok=True)
    
    # Copy the source file to the target file location
    shutil.copy2(source, target)
    print("File copied successfully.")
 