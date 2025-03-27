virtual std::vector<ermy::u8>&& GetStaticPreviewData() { 
	static std::vector<ermy::u8> empty;
	return std::move(empty); 
}; 