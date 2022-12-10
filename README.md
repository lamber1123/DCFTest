# DCFTest: 分布式一致性框架的测试工具
&#160; &#160; &#160; &#160; DCFTest基于GitHub上的项目DCF-Demo，希望通过设计DCF测试程序来验证DCF的可用性与性能，便于后期和其他的分布式项目对接。

DCF-Demo项目地址：https://github.com/NPUWDBLab/DCF-Demo

DCFTest实现了基于DCF的下列测试情景:
 * **集群选举测试**: DCFTest可以通过指令组合，实现集群节点与配置的动态变更。用以对容灾性自动与手动升降主备，集群状态查询，集群配置管理情景的测试。
 * **日志复制测试**: DCFTest在程序内部提供了交互窗口，允许用户通过FCFTest所支持的读写指令对DCF进行读写数据。在不同的集群情境下，测试跨可用区的多副本复制。
 * **集群故障测试**: DCF作为稳定可靠的一致性数据复制组件，为了测试DCF在任何条件下的可用性，DCFTest提供了可以模拟多种集群故障情况的指令。通过不同的指令组合，用户可以测试DCF在节点宕机、网络分区等情况下的可用性情况。

---

## 在DCF-Demo之上做的工作

### 1. DCF-Demo的封装

&#160; &#160; &#160; &#160; DCFTest希望能够通过DCF-Demo中的代码来进行测试，所以首要工作是从对DCF-Demo中的部分代码段进行封装，来供DCFTest调用。在封装DCF-Demo之后，通过DCF-Demo对DCF的测试变得更简单了。另一方面，提出接口更加方便测试。

![DCF-Test与DCF-Demo间的联系](./imgs/Figure1.png)

### 2. 添加网络传输能力

接下来解决的问题是，目前的DCFTest只能针对当前服务器的节点进行测试。如果想对集群的一致性和可用性进行测试，需要同时在多台服务器上进行操作。这样既不高效也不优雅，无法进行批量操作。因此，我在DCFTest中增加了网络传输模块（msg），通过tcp协议，将不同节点上的结果收集到一个节点上，来支持后续的批量测试工作。

![为DCFTest添加网络传输模块](./imgs/Figure2.png)

---

## 如何安装与运行DCFTest

### 1. 外部依赖关系

&#160; &#160; &#160; &#160; 在运行DCFTest之前，需要确保安装配置好了下面列出的依赖项。
* DCF 3.1.0 
* gcc 4.8.5

### 2. 配置

* 将DCFTest安装到本地
```javascript
# clone DCFTest to local
[lenyb@slave02 DCFTest]$ git clone https://github.com/lamber1123/DCFTest.git
```
* 修改“DCFTest/DCFTestConfig.json”中的集群配置，将各集群节点对应的node_id、IP与port字段进行修改
```javascript
# set the node_id, IP and port
[lenyb@slave02 DCFTest]$ vim DCFTestConfig.json
```
* 设置环境变量，将LD_LIBRARY_PATH设置为DCF生成的lib库路径
```javascript
# set environment variable
[lenyb@slave02 DCFTest]$ export LD_LIBRARY_PATH=//data/toolchain/lib::$LD_LIBRARY_PATH
```

### 3. 运行

**所有节点所在的服务器上均要配置DCFTest**
* 运行build.sh以启动DCF
```javascript
# run DCFTest
[lenyb@slave02 DCFTest]$ sh build.sh
```

![DCF-Demo在GitHub上的仓库](./imgs/Figure3.png)

### 4. 使用方法

* 在终端交互窗口中输入指令
* 如果提示PASSED，代表操作成功，如果提示FAILED，则代表操作失败
* 输入错误的指令或输入空指令后，会对程序支持的指令进行提示

**至此，DCFTest已经部署完成**

### 5. 测试

&#160; &#160; &#160; &#160; 详细的测试过程将在<a href="https://lamber1123.github.io/2022/12/07/DCFTest%EF%BC%9ADCF%E6%B5%8B%E8%AF%95%E6%A1%86%E6%9E%B6%E7%9A%84%E8%AE%BE%E8%AE%A1/">《DCFTest：DCF框架的测试工具》</a>中记录。

