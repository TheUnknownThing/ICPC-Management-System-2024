# ICPC Toy Management System Complexity Analysis

## 总体复杂度分析

通过kcachegrind检索出了函数的运行时间，其中：
- `Submit`函数占用时间最多，约占总时间的30%，但是，其中的大头其实是输入输出操作，而非函数本身。
- `Scroll`函数占用时间约为25%，但其中16%/25%是调用`PrintScoreboard`函数，因此实际上`Scroll`函数的时间复杂度并不高。也意味着`PrintScoreboard`函数的优化空间很大。
- 另外，`QueryRanking`函数占用时间约为20%，复杂度主要来自于字符串的比较(占函数时间的90%)。


其中最耗时的操作是榜单解冻，因为需要处理所有冻结的提交并重新计算排名。

## 数据结构复杂度
首先分析主要使用的数据结构：
- `std::set<Team_Set, comp>`: 基于红黑树，插入/删除/查找复杂度为 $O(\log n)$
- `std::unordered_map`: 平均O(1)的查找/插入/删除复杂度
- `std::multiset`: 基于红黑树，插入/删除/查找复杂度为 $O(\log n)$
- `std::vector`: 随机访问O(1)，尾部插入平均O(1)，中间插入O(n)，`find`操作O(n)，`lower_bound`操作$O(\log n)$

## 函数复杂度分析

### 约定记号
- $n$: 队伍总数
- $p$: 题目总数 (p <= 26)
- $s$: 队伍提交总数
- $f$: 冻结提交总数

### `AddTeam`
- 时间复杂度: $O(\log n)$，因为执行了查找队伍是否已经存在的操作。

### `Flush`
- 时间复杂度: $O(n)$。在平时的提交和滚榜中，就已经维护好了`team_set`，因此`Flush`函数的操作不过是把`team_set`中的队伍顺序插入到`team_rank`中。
- 主要操作：刷新`rank`，并不会对`set`进行操作
- 时间复杂度被平摊到了`Submit`和`Scroll`函数中。

### `Submit`
- 有以下几种情况：
  - 题目之前已经被解决，时间复杂度为O(1)
  - `frozen`状态，插入到`frozen_submissions`中，时间复杂度为$O(\log f)$，这个也可以认为是常数时间。
  - 未被冻结状态，但是题目未被解决，插入到`team_submission`中，时间复杂度为O(1)
  - 未被冻结状态，题目已经被解决，插入到`team_submission`中，计算罚时，维护`team_set`，时间复杂度为$O(\log n)$再加上`CalculateProblemTime`的时间复杂度。

  其中`Submit`调用的子函数：

  #### `AddTeamTime`
  - 时间复杂度: $O(\log p)$，其中`p`为题目总数，可以认为是常数（$p \leq 26$）
  - 为什么有这个函数？因为`set`的比较规则有一条要比较题目通过的时间。
  - 主要操作：
    - lower_bound: $O(\log p)$
  
  #### `CalculateProblemTime`
  - 这个其实挺耗时间，因为要遍历队伍所有的提交，时间复杂度为O(s)

### `Scroll`
- 总体分析：`Scroll`函数的时间复杂度主要来自于`PrintScoreboard`函数，以及其中维护`team_set`的操作。
- 维护`team_set`：常规的操作就是先`erase`再`insert`，这里不再赘述了。查找“替换的队伍”其实是一个$O(\log n)$的操作，我们做的操作是在 `erase` 之前取出当前队伍的后一名名字，然后再 `insert` 之后再取出当前队伍的后一名名字。比较二者是否相同，若不相同，则说明当前队伍的名次发生了变化。
- `PrintScoreboard`函数：主要操作是遍历`team_rank`，然后对于每个队伍输出每道题的状态。时间复杂度为$O(n \cdot p)$
- 最终，`Scroll`函数需要执行一次`Flush`，两次`PrintScoreboard`，以及最多$n \times p$次的维护`team_set`操作。
- 也即，`Scroll`函数的时间复杂度为$O(np \log n) = (n \log n)$ (p我们认为是常数)


### `QueryRanking`
- 时间复杂度: $O(n)$，因为是在`team_rank`这个`vector`中查找

### `QuerySubmission`
- 最坏情况下时间复杂度为$O(s)$，因为要遍历队伍的所有提交。
- 最好情况下时间复杂度为O(1)，也即STATUS=ALL and PROBLEM_NAME=ALL 直接返回队伍的最后一次提交。

