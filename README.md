# system-test


destination file: burstt2:/mnt/data/fpga2.mt
以上的目錄都已經用 hugepage 宣告過了。
利用 multithread 把 source file 複製到 destination file，並用 time 計時。底下是 8 個 thread 的例子。
$ echo 3 | sudo tee /proc/sys/vm/drop_caches ; sudo rm /mnt/data/fpga2.mt ; time sudo (taskset -c 35-50 有加沒加這個指令沒有太大差別) ./wrb_mt -pthread 8
程式會把 source file 分成 8 個部分，每個部分用 1 個 thread 負責拷貝。
程式每次的執行時間會有些差異，但大多是 3 秒多 (real)。執行 wrb_mt 程式期間，rudp 程式並沒有執行。

拷貝 66 GB 的檔案最快也要花掉 3.xx 秒，無法再快了。 

