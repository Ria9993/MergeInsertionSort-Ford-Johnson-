# Summary
C++ implementation of merge-insertion sort(Ford-Johnson algorithm)  

# Reference
- L. Ford, S. Johnson, A tournament problem, American Mathematical Monthly 66 (1959) 387–389 [<https://doi.org/10.2307/2308750>]  
- Knuth, Donald E. The Art of Computer Programming: Volume 3: Sorting and Searching. Addison-Wesley Professional; 2 edition (May 4, 1998).  

## Blog  
- Jacobsthal sequence의 closed form 유도 [<https://ria9993.github.io/math/2024/03/05/Jacobsthal-sequence.html>]
- 올바른 퍼포먼스 테스트를 위한 캐시 플러쉬 [<https://80000coding.oopy.io/0fb3911b-d066-455e-9a65-d61b7b6671c4>]  

## Note
```
정렬의 하한은,
n개의 원소가 있다면 해당 원소들의 가능한 정렬 결과 경우의 수는 n! 이므로,
Balanced Binary tree로 생각한다면 최대 비교횟수는 ceil(log_2(n!)) 과 같다.
Log_2(n!) 는 stirling's approximatation 에 따르면 O(n log n)임.
이제 정렬의 하한에 가까운 비교횟수를 가진 알고리즘을 생각해보자면...
Binary insertion 으로 n개의 원소를 정렬하려면 SUM[k = 1, k -> n]  ceil(log k) 의 비교횟수가 필요함.
이는 정렬 하한에 그닥 근접하지 않음.
왜냐하면 기본적으로 ceil(log k)로 계산해야 하므로 이산적인 낭비가 있음.
log k의 그래프는 부드럽지만 ceil(log k)는 부드럽지 않음,
그러므로 이 낭비를 최소화 할 것임.
즉 binary insertion을 할 때 최소 손실인 2^n - 1개를 비교하도록 할 것.
->->
어쨋든 여기서 많이 유도하다 보면
f(n) = n번의 비교횟수가 필요한 index
2^n - 1 = 2f(n - 1) + (f(n) - f(n-1) - 1)
2^n = f(n - 1) + f(n)
2^(n-1) = f(n - 2) + f(n - 1)
2^n = 2f(n - 2) + 2f(n - 1)
2f(n - 2) + 2f(n - 1) = f(n - 1) + f(n)
f(n) = f(n - 1) + 2f(n - 2)
이렇게 하면 귀납식이 나오는데 피보나치랑 비슷한 이런 걸 lucas sequence 라고 해요.
Jacobsthal sequence랑 똑같아서 그냥 그렇게 씁니다.
이제 insert할 인덱스를 구할 수 있겠네요.
이제 FJ 알고리즘 전체 비교횟수를 계산해볼까요
FJ알고리즘의 전체 비교횟수는
F(n) = floor(n / 2) + F(floor(n / 2)) + G(ceil(n / 2))
로 볼 수 있는데
G는 다음과 같이 계산할 수 있겠죠
(but, f(k - 1) < m <= f(k))
G(m) = SUM[j=1, j -> k-1] { j * (f(j) - f(j - 1)) } + (k * (m - f(k - 1)))
Telescope sum...
SUM[j=1, j -> k-1] { j * (f(j) - f(j - 1)) }
=
f(1) - f(0) +
2f(2) - 2f(1) +
3f(3) - 3f(2) +
...
(k - 1)f(k - 1) - (k - 1)f(k - 2)
그러므로
G(m)
= SUM[j=1, j -> k-1] { j * (f(j) - f(j - 1)) } + (k * (m - f(k - 1)))
= -(f(0) + f(1) + f(2) + ... + f(k - 2)) + (k - 1) * f(k - 1) + km - kf(k - 1)
= -(f(0) + f(1) + f(2) + ... + f(k - 2)) - kf(k - 1) + km
= km - (f(0) + f(1) + f(2) + ... + f(k - 1))
더 계산하기 위해서 위에서 유도했던 수열 점화식을 생성함수나 고유벡터등으로 일반화 하면
f(k) = (2^k - (-1)^k) / 3
(but, f(k) + f(k - 1) = 2^k)
이 나오는데
(제가 정리해둔게 있어요 https://ria9993.github.io/cs/2024/03/05/Jacobsthal-sequence.html)
일단 위에서 구한 일반항은 0 1 1 3 5 11.. 이렇게 나오는 수열이었는데
우리는 1-based 인덱스로 수열 1 1 3 5 11... 이 나오게 할 것이라
일반항을 약간 수정
f(k) = (2^(k + 1) + (-1)^(k)) / 3
이제 일반항으로 다음을 구해보죠...
SUM[j=0, j -> k - 1] f(j)
= f(0) + f(1) + f(2) + ... + f(k - 1)
= (1/3) * SUM[j=0, j -> k - 1] (2^(j + 1) + (-1)^j)
SUM[j=0, j -> k - 1] (2^(j + 1) + (-1)^j)
= f(0) + f(1) + f(2) + ... + f(k - 1)
=
2^1 + (-1)^0 +
2^2 + (-1)^1 +
2^3 + (-1)^2 +
...
2^k + (-1)^(k - 1)
2^1 + 2^2 + 2^3 + ... + 2^k = 2^(k + 1) - 2
(-1)^0 + (-1)^1 + (-1)^2 + ... + (-1)^(k - 1) = (1 + (-1)^k) / 2
이므로
f(0) + f(1) + f(2) + ... + f(k - 1)
= (2^(k + 1) - 2 + (1 + (-1)^k) / 2)
인데
좀 간단하게 하기 위해 조금 돌아가서...
f(n) = (2^(n + 1) + (-1)^n) / 3
위 일반항에서 -1 의 거듭제곱은 3으로 나누어 떨어지게 해주는 역할을 한다.
(2와 3은 서로소)
2의 짝수 거듭제곱은 3으로 나누었을 때 나머지가 1이고,
홀수 거듭제곱은 3으로 나누었을 때 나머지가 2이다.
그러므로 (-1)^n 은 3으로 나누어 떨어지게 해주는 역할을 한다.
하지만 (-1)^0 + (-1)^1 + (-1)^2 + ... + (-1)^(n) 은 0 혹은 1이다.
그러므로
f(0) + f(1) + ... f(k - 1)
= (2^(k + 1) - 2 + [0 or 1]) / 3
= (2^(k + 1) - [2 or 1]) / 3
여기서 -2냐 -1이냐에 따라서 결과가 달라질 수 있냐를 생각해야 한다.
2 ^ (k + 1)은 3으로 나누었을 때 나머지가 1 혹은 2인데
여기에 2나 1을 뺀다고 해서 3으로 나눈 몫을 내림한 값은 달라지지 않는다.
그러므로 (-1)^n은 생략 가능하다.
f(0) + f(1) + ... f(k - 1)
= floor(2^(k + 1) / 3)
이제 G(m)를 구할 수 있다.
G(m) = km - floor(2^(k + 1) / 3)
이제 F(n) 을 구하러 가볼 건데
f(k - 1) < m <= f(k) 이므로
w(k) = f(0) + f(1) + ... f(k - 1) = 2^(k + 1) / 3 로 정의하면
w(k) < n <= w(k + 1) 일 때
F(n) - F(n - 1) = k 임을 보일 수 있다.
직관적으로 설명하면 m번째 원소는 삽입할 때 k번의 비교가 필요하고,
G(m)의 계산식을 보면 f(k - 1) + 1 부터 f(k) 까지의 원소들의 비교횟수 k들의 합이다.
G(m)의 계산식
SUM[j=1, j -> k-1] { j * (f(j) - f(j - 1)) } + (k * (m - f(k - 1)))
에서 SUM 부분은 1번째부터 f(k - 1)번째 원소들의 비교횟수의 합이었는데
이것을 정리한 식을 보면
-(f(0) + f(1) + f(2) + ... + f(k - 2)) + (k - 1) * f(k - 1)
이런 식이었고 이걸 직관적으로 생각해보면
1번째 원소부터 f(k - 1)번째 원소까지의 k - 1번 비교한다고 생각했을때의 비교횟수의 합 ((k - 1) * f(k - 1))에
f(0) + f(1) + ... + f(k - 2) 를 뺀 것이라고 생각할 수 있다. (대충 생각해보면 맞다 f = { 1, 1, 3, 5, 11 ...})
그러므로 재밌는 건 f(0)부터 f(k - 1) 까지의 합이 f(k)보다 작다는 것이다.
그러니 w(k + 1) - w(k) = f(k) 이고 w(k) < n <= w(k + 1) 가 성립.
그렇다면 F(n) - F(n - 1) = k 일 것이다.
w(k) < n <= w(k + 1)에
w(k) = 2^(k + 1) / 3 을 대입하면
2^(k + 1) / 3 < n <= 2^(k + 2) / 3
나눠서 정리
2^k / 3 < n / 4 <= 2^(k + 1) / 3
2^(k - 1) < 3n / 4 <= 2^k
k - 1 < log_2(3n / 4) <= k
k = ceil(log_2(3n / 4))
or k = floor(log_2(3n / 4)) + 1도 되는데 이건 실수가 아니라 정수라서 안됨
그러므로 포드존슨 알고리즘의 전체 비교횟수는 다음과 같음.
F(n) - F(n - 1) = ceil(log_2(3n / 4))
F(n) = SUM[k = 1, k -> n] ceil(log_2(3k / 4))
그리고 이는 n <= 22 일때 정렬의 하한 ceil(log_2(n!)) 와 같다.
(꽤 근사함)
```

