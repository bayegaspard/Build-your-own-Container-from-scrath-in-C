# Create Your Own Container using C
### Author : Baye Gaspard
My complete source code and related files can be found here
###  Create child process
- To create a child process we have to create a child function with its own `pid` such that the child thinks that there is no other process runing.We do that by calling the `clone()` function with the  `CLONE_NEWPID` flag.We can also use a different system call called `unshare` to do similar task.Once we call `clone()`,  is called with the flag added, the new process  starts within a new `PID namespace`, under a new `process tree`.
- We can add many flags to the clone() function depending on what we want to be isolated from the containter point of view.
```
 int clone_flags = CLONE_NEWUSER|CLONE_NEWPID|CLONE_NEWNS|CLONE_NEWNET|SIGCHLD;
```
### Isolate it using namespaces
- This new process is isolated using the `CLONE_NEWPID` flag.this helps the process to see only this pid.
- A process which is isolated from the other process will have its own `pid` to be `1`.

- Note that `clone()` is a system call and need root priviledge to execute the binary after compiling using the following command:
```
gcc -o pid -w pid.c
```
- Then we can run the binary with sudo priviledge.

![](https://i.imgur.com/RbRJdIR.png)

- I supplied `flags` for this PoC, the flag `CLONE_NEWUTC` is for container `hostname`, where when `clone()` is called it allow the container have its own hostname as in the snippet below.I also added some `C` code to give the container a custom name.
```
 // sethostname
        const char * new_hostname = "my container";
        if (sethostname(new_hostname, strlen(new_hostname)) != 0) {
                fprintf(stderr, "failed to execvp argments %s\n",
                        strerror(errno));
                exit(-1);
        }

```

![](https://i.imgur.com/EjFxBOd.png)
- Meanwhile on the other hand the flag `CLONE_NEWUSER` enable the container to handle its own user and group.In this case since we did not create one , it will have `nobody` as user as seen below.
![](https://i.imgur.com/JYbgkLz.png)

- For Networking , I used the flag `CLONE_NEWNET`, which allow the container to look only it's own network interface. It is usually loopback if there are no other explicitely set up network interface.
- Below we can see the network interface for the child process with respect to that of the main host.
![](https://i.imgur.com/bB5MFri.png)

- But for us to make the two processes , parent and child process each manage thier network interfaces, we need to create a  pair of virtual Ethernet connections  using the following command:
```
ip link add name veth0 type veth peer name veth1 netns <pid>

```
- Where `<pid` is the process id of the child namespace, `veth0` device is for the parent namespace and `veth1` device for the child namespace device.We enter new namespace using the `netns`.
- For example for the `pid 4` found in `/proc` , when I use the command, it created and shows that it exist since I want to do it again.
![](https://i.imgur.com/dQDxPSn.png)
![](https://i.imgur.com/MRJ8zmc.png)

- For mount point, we need to use the flag `CLONE_NEWNS` and precise a mounting point.This mounting point have been handled using `C` via the following code snippet.
```

        // mount proc
        if (umount("/proc", 0) != 0) {
                fprintf(stderr, "failed unmount /proc %s\n",
                        strerror(errno));
                exit(-1);
        }
        if (mount("proc", "/proc", "proc", 0, "") != 0) {
                fprintf(stderr, "failed mount /proc %s\n",
                        strerror(errno));
                exit(-1);
        }

```
- The easiest way to mount `proc` can simply be using the command : 
```
mount proc /proc -t proc
```
- In some cases, `chroot()` can also be used as well.
- We need to create a virtual disk using the loop tool.This will host the file system of our containter.
- We use the following commands:
- Using `dd` command I create 10G of zerod file.i.e full with zeros.
```
dd if=/dev/zero of=files.fs bs=1024 count=10000000

```
- Then I used the command `losetup -f` to tell me which `/dev/loop` is free.
- Next, `losetup  /dev/loop1 file.fs` 
- We use the `ext4 file system` via the command below using the tool `mkfs` : 
```
mkfs -t ext4 /dev/loop1 
```
- We mount to the `mnt` directory using the command :
```
mkdir mnt
mount -t ext4 /dev/loop1 /mnt
``` 
- We verify using the `mount | grep mnt`
![](https://i.imgur.com/fGCVqj9.png)

### Benchmark [ Your container, host machine, LXC, Docker ]
- I benchmarked on ` cpu, memory, fileio, threading` using the following commands respectively:
```
sysbench --test=cpu --cpu-max-prime=2000000 --num-threads=120 run

```
```
sysbench --test=memory --num-threads=140 --memory-total-size=10G run
```
```
sysbench --num-threads=16 --test=fileio --file-total-size=10G 
--file-test-mode=rndrw prepare
```
```
sysbench --test=threads --thread-locks=10 --max-time=60 run

```

# References
- https://www.toptal.com/linux/separation-anxiety-isolating-your-system-with-linux-namespaces
- https://www.howtoforge.com/how-to-benchmark-your-system-cpu-file-io-mysql-with-sysbench
- https://www.sciencedirect.com/science/article/pii/
- https://minervadb.com/index.php/2018/03/27/benchmarking-cpu-memory-file-i-o-and-mutex-performance-using-sysbench/
- https://wiki.gentoo.org/wiki/Sysbench
