void *memcpy(void *dest, const void *src, size_t n) {
	for (int i = 0; i < n; ++i) {
		((char *)dest)[i] = ((char *)src)[i];
	}
	return dest;
}
