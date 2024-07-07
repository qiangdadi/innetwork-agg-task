# Working Record
- Day 1: build n3s-with QUIC. 
    - Learn how to use quic in ns3.
    - understand the task, read related papers.
    - know what is MPI.
- Day 2：try to implement a very very simple MPI.
    - know what is communicator in MPI.
    - know these basic operator in MPI: BroadCast, Reduce, AllReduce.
    - create the structure of innetwork-train.
- Day 3: an annoying bug.
    - implement producer, consumer with QUIC.
    - bug : nodes can not communicate each other.
      I try to locate this bug within one day. Finally I find it it very very funny and annoying. 
      When I attach the drop rate to p2p link, this bug happen. But When I annotate them, everything is fine. 
- Day 4-8: Learning to use ns3 and implement my own protocol (according to [switchML](https://www.usenix.org/system/files/nsdi21-sapio.pdf) and [sharp](https://network.nvidia.com/related-docs/solutions/hpc/paperieee_copyright.pdf)).
    - implement simpleMPI.
    - design the pactek format
- Day 9-12: debuging. 
    - the underlying quic protocol implemented in ns3 is not stable. It has some bugs.
    - So I had to work on these bugs. Some has been fixed but some still exists.
- Day 13-15: change another quic implementation in ns3.
    - all bugs in quic-ns3 have been fixed.
    - test it with many tests. 

## File description
- scratch/in-quic-topo.cc : create topology and run simulation
- src/innetwrok-train/ : new ns3 module to simlate producers, consumer, and aggrgators
- topofile store topo infomation
- testscript.ipynb : the jupyter script to run the simulation
- quic : the revised quic to support this task

## fixed bugs
- simulation interrupted unexpectedly (30 producers and 1 consumer without aggregator) --fixed.

  Memory access out-of-band-->enlarge allocated memory.
  
- 30 producers and 1 consumer without aggregator cannot end (25 producers works ok)

  Due to packet lost, transmission has some [issues](https://github.com/signetlabdei/quic-ns-3/issues/41). try to fixed.
  But maybe another problem occurs. I have report these bugs to the offical ns3-quic group.
 
- I try to use ns3 3.37 and the corresponding [quic-ns3 3.37](https://github.com/signetlabdei/quic) implementation. But it seems also doesn't work so well.

   So I try to use another [gquic-ns3 3.33](https://github.com/SoonyangZhang/quic-on-ns3) implementation, gquic. But it may be need time to reconstruct my code.

- [gquic-ns3 3.33](https://github.com/SoonyangZhang/quic-on-ns3) is a robust quic in ns3.


## two bugs and temporal solutions
- ~~buffer.h ("m_current >= m_dataStart && m_current < m_dataEnd", msg="You have attempted to read beyond the bounds of the available buffer space.)
  solution : enlarge stream buffer size. (this bug just exists in ns3.33. And this bug has been fixed by ns3 3.37.)~~

- ~~multistream support 
  current quic version just support 2 streams (0 and 1).~~

## Protocol Working Process
- consumer send request to all his children node.
- then the children node (maybe producer or aggregator) will try to forward the request to its children.
- the leaf node will generate a random vector and then cut the data into blocks with k elements. We use offset to locate the right position in the original vector.
- aggregator node collect all the data and then forward the result to its parent 
- root will save the sesult and start a new request.

## Notes
- Due to the bug in [quic-ns3 3.37](https://github.com/signetlabdei/quic). I do not add loss rate to the p2p link in this version. Later I will try to find out the bug and solve it or just use another [gquic-n3 3.33](https://github.com/SoonyangZhang/quic-on-ns3).

- ~~The simulation process will end at 632 iteration in ns3 3.37 (ns3 3.33 seems works ok). Someone report the [issue](https://github.com/signetlabdei/quic-ns-3/issues/20) but seems no one answer. Later I will reconstruct the code structure to fix it. (for example, the whole simulation will be triggered by producers rather than consumer.)~~
