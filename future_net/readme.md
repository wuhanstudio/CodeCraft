1.针对不同规模的点数我们采用了不同的算法优化,不同的算法对数据存储方式进行了不同的优化，split破坏了*graph[],
  于是search_route我们另外传入了topo_file，被破坏后重新读取graph
