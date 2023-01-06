import os
pid = int(input("PID: "))
struct_id = int(input("Select struct:\n 0 - dentry\n 1 - task_cputime\n"))
os.system("echo '%d %d' > /proc/lab2" % (pid, struct_id))
os.system("cat /proc/lab2 2>/dev/null")
