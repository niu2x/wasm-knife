# Now, this is only useful for me

	let asm = GameGlobal[0].Module.asm;
	let len = 4*1024*1024;
	let ptr = asm.malloc(len)
	let outLen = asm.__write_profile(ptr, len)
	let profile = asm.memory.buffer.slice(ptr, ptr+outLen)
	wx.getFileSystemManager().writeFileSync(wx.env.USER_DATA_PATH + '/my.profile', profile)
	asm.free(ptr)