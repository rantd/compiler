int mod_pow(int x, int y, int mod)
{
    int res;
    int a;
    res = 1;
    a = x;
    while (y > 0) {
        if (y % 2 == 1) {
            res = (res * a) % mod;
        }
        a = (a * a) % mod;
        y = y / 2;
    }
    return res;
}

int comb(int k, int n, int mod)
{
	int i;
	int tk;
	int tn;
	int tnk;
	int s;
	if (k > n) return 0;
	if (k == n) return 1;
	if (k == 0) return 1;
	s = 1;
	i = 1;
	while (i <= n) {
		s = (s * i) % mod;
		if (i == k) tk = s;
		if (i == n) tn = s;
		if (i == n - k) tnk = s;
		i = i + 1;
	}
	s = tn;
	s = (s * mod_pow(tk, mod - 2, mod)) % mod;
	s = (s * mod_pow(tnk, mod - 2, mod)) % mod;
	return s;
}

int c0func(int k, int n, int mod, int a, int b, int c)
{
	int tk;
	int tn;
	int mk;
	int mn;
	int ans;
	tk = k;
	tn = n;
	ans = 1;
	while (1) {
		mk = tk % mod;
		mn = tn % mod;
		ans = (ans * comb(mk, mn, mod)) % mod;
		tk = tk / mod;
		tn = tn / mod;
		if (tn + tk == 0) break;
	}
	print_int(ans);
  	print_line();
  	return 0;
}

