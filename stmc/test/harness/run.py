import subprocess

tests = [ "integer_arithmetic" ]

try:
    for test in tests:
        result = subprocess.run(
            ['./' + test], 
            capture_output=True, 
            text=True)
        
        print("code:", result.returncode)
        print("stdout:", result.stdout)

except subprocess.CalledProcessError as err:
    print(f"Error running test: {err}")

except FileNotFoundError:
    print("Executable for test not found.")
