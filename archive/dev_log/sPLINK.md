## Step1: Filter and Select

do any custom filter at client.

#### Collect SNPs

SNP names are not private data so they can be sent to the central server without an enclave. 
collect SNP names at the central server -> compare and find common SNPs -> send back to client and select common SNPs


## Step2: Aggregation

compute metadata like sample count (sum of `y.count()`), allele count, minor allele mapping etc. 

divide data into chunks for parallel computing

> client --> enclave: metadata(row identifiers, in most cases alleles)

## Step3: Association Test

#### Association

#### Linear Regression

as define by `linear_regression_rows` in hail

==local==: $X_i^TX_i$ and $X_i^T Y$ of each row

> client --> enclave: (if there are 2 covariant like our sample) a 3x3 matrix and a 3 dim vector per client per row

**==enclave==**:$X^TX=\sum X_i^TX_i$ and $X^TY=\sum X_i^T Y$ .

calculate $\beta = (X^TX)^{-1} X^TY$ of each row 

> enclave --> client: one $$\beta$$ per row 

==local==: $SSE_i=\sum(Y_i - \hat{Y_i})^2$ (only ONE double per client) -> public

> client --> enclave: one value per client per row

==**enclave**==: standard error $S=\sum(SSE_i)/(n-p-1)$ -> ==$S$ is public== (is this safe? )

 $SE_1=\sqrt{S (X^TX)^{-1}_{11}}$ 

$t=\beta_1/SE_1$, p-value

#### Logistic Regression

