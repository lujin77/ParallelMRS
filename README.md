# ParallelMRS
a Hadoop stream like parallel processing framework on multicore platform
通过借鉴分布式环境下的MapReduce模型，实现了一个多核并行数据处理框架，该框架借助子进程与匿名管道技术，可通过类似Hadoop Streaming的形式与任意基于标准IO的程序、脚本进行集成，并自动以并行方式调度其执行。大大提升了多核平台上，单机数据处理的效率和便利性。
