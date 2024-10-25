import subprocess
import filecmp
import os
import shutil
from termcolor import colored


# 比较输出文件和预期输出文件，返回不匹配的行
def compare_output(output_file, expected_output_file):
    with open(output_file, 'r') as outfile, open(expected_output_file, 'r') as expfile:
        output_lines = [line.strip() for line in outfile.readlines()]
        expected_lines = [line.strip() for line in expfile.readlines()]

    mismatched_lines = []
    for i, (out_line, exp_line) in enumerate(zip(output_lines, expected_lines)):
        if out_line != exp_line:
            mismatched_lines.append((i + 1, out_line, exp_line))

    return mismatched_lines

if __name__ == "__main__":
    testcases_dir = 'data/'
    for filename in os.listdir(testcases_dir):
        if filename.endswith('.in'):
            print(f"Running test {filename}...")
            test_num = filename.split('.')[0]
            input_file = os.path.join(testcases_dir, f'{test_num}.in')
            output_file = 'output.txt'
            expected_output_file = os.path.join(testcases_dir, f'{test_num}.out')

            # 拷贝输入文件到 input.txt
            shutil.copy(input_file, 'input.txt')

            subprocess.run(['./main'])
            
            mismatches = compare_output(output_file, expected_output_file)

            if not mismatches:
                print(colored(f"Test {test_num} passed.", 'green'))
            else:
                print(colored(f"Test {test_num} failed: The output does not match the expected output.", 'red'))
                for line_num, out_line, exp_line in mismatches:
                    print(f"Line {line_num}:\nOutput: {out_line}Expected: {exp_line}")
                
                os.remove('input.txt')
                os.remove('output.txt')    
                exit(1)

            os.remove('input.txt')
            os.remove('output.txt')