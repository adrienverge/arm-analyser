int one_arg(int val)
{
	return val + 1;
}
int two_args(int val1, int val2)
{
	return val1 + val2;
}
int three_args(int val1, int val2, int val3)
{
	return val1 * val2 - val3;
}
int test(int val)
{
	if (val > 5)
		return one_arg(val);
	else if (val < 0)
		return two_args(42, val);
	return 9;
}
void operations(int val)
{
	one_arg(val++);
	two_args(42, val);
	three_args(87, 88, 89);
}
int main(int argc, char **argv)
{
	int a = test(argv[0][0]);
	operations(a);
	return a;
}
