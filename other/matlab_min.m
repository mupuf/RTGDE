start = tic();

sum = 0;
sum_sq = 0;
count = 0;

while (toc(start) < 30)
	tic();
	usleep(1000);
	time = toc() * 1e6 - 1000;

	sum += time;
	sum_sq += time * time;
	count = count + 1;
endwhile

avr = sum / count;
avr_sq = sum_sq / count;
std = sqrt(avr_sq - (avr * avr));
printf("sleep jitter: avr = %+.3f µs, std = %.3f µs\n", avr, std);