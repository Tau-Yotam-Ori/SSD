
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
//#include <string.h>
#include <sys/ioctl.h>
//#include <sys/stat.h>
//#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
//#include <util.h>

#define nvme_admin_cmd nvme_passthru_cmd

// #define _IOC(dir,type,nr,size) \
// 	(((dir)  << _IOC_DIRSHIFT) | \
// 	 ((type) << _IOC_TYPESHIFT) | \
// 	 ((nr)   << _IOC_NRSHIFT) | \
// 	 ((size) << _IOC_SIZESHIFT))

#define _IOC_TYPECHECK(t) (sizeof(t))

//#define _IOWR(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define NVME_IOCTL_ADMIN_CMD	_IOWR('N', 0x41, struct nvme_admin_cmd)


#define NVME_NSID_ALL		0xffffffff
#define OPT_ARGS(n) \
	const struct argconfig_commandline_options n[]

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#else
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#endif

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#define __bitwise __bitwise__


#ifdef __CHECKER__
#define __force       __attribute__((force))
#else
#define __force
#endif

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;

typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;

struct nvme_smart_log {
	__u8			critical_warning;
	__u8			temperature[2];
	__u8			avail_spare;
	__u8			spare_thresh;
	__u8			percent_used;
	__u8			endu_grp_crit_warn_sumry;
	__u8			rsvd7[25];
	__u8			data_units_read[16];
	__u8			data_units_written[16];
	__u8			host_reads[16];
	__u8			host_writes[16];
	__u8			ctrl_busy_time[16];
	__u8			power_cycles[16];
	__u8			power_on_hours[16];
	__u8			unsafe_shutdowns[16];
	__u8			media_errors[16];
	__u8			num_err_log_entries[16];
	__le32			warning_temp_time;
	__le32			critical_comp_time;
	__le16			temp_sensor[8];
	__le32			thm_temp1_trans_count;
	__le32			thm_temp2_trans_count;
	__le32			thm_temp1_total_time;
	__le32			thm_temp2_total_time;
	__u8			rsvd232[280];
};

struct nvme_passthru_cmd {
	__u8	opcode;
	__u8	flags;
	__u16	rsvd1;
	__u32	nsid;
	__u32	cdw2;
	__u32	cdw3;
	__u64	metadata;
	__u64	addr;
	__u32	metadata_len;
	__u32	data_len;
	__u32	cdw10;
	__u32	cdw11;
	__u32	cdw12;
	__u32	cdw13;
	__u32	cdw14;
	__u32	cdw15;
	__u32	timeout_ms;
	__u32	result;
};

enum nvme_print_flags {
	NORMAL	= 0,
	VERBOSE	= 1 << 0,	/* verbosely decode complex values for humans */
	JSON	= 1 << 1,	/* display in json format */
	VS	= 1 << 2,	/* hex dump vendor specific data areas */
	BINARY	= 1 << 3,	/* binary dump raw bytes */
};

enum {
	NVME_LOG_SMART		= 0x02,
    NVME_NO_LOG_LSP       = 0x0,
    nvme_admin_get_log_page		= 0x02
};

const char *devicename;

static long double int128_to_double(__u8 *data)
{
	int i;
	long double result = 0;

	for (i = 0; i < 16; i++) {
		result *= 256;
		result += data[15 - i];
	}
	return result;
}


// static inline uint32_t le32_to_cpu(__le32 x)
// {
// 	return le32toh((__force __u32)x);
// }

void nvme_show_smart_log(struct nvme_smart_log *smart, unsigned int nsid,
			 const char *devname, enum nvme_print_flags flags)
{
	/* convert temperature from Kelvin to Celsius */
	int temperature = ((smart->temperature[1] << 8) |
			    smart->temperature[0]) - 273;
	int i, human = flags & VERBOSE;

	printf("Smart Log for NVME device:%s namespace-id:%x\n", devname, nsid);
	printf("critical_warning			: %#x\n",
		smart->critical_warning);

	if (human) {
		printf("      Available Spare[0]             : %d\n", smart->critical_warning & 0x01);
		printf("      Temp. Threshold[1]             : %d\n", (smart->critical_warning & 0x02) >> 1);
		printf("      NVM subsystem Reliability[2]   : %d\n", (smart->critical_warning & 0x04) >> 2);
		printf("      Read-only[3]                   : %d\n", (smart->critical_warning & 0x08) >> 3);
		printf("      Volatile mem. backup failed[4] : %d\n", (smart->critical_warning & 0x10) >> 4);
		printf("      Persistent Mem. RO[5]          : %d\n", (smart->critical_warning & 0x20) >> 5);
	}

	printf("temperature				: %d C\n",
		temperature);
	printf("available_spare				: %u%%\n",
		smart->avail_spare);
	printf("available_spare_threshold		: %u%%\n",
		smart->spare_thresh);
	printf("percentage_used				: %u%%\n",
		smart->percent_used);
	printf("endurance group critical warning summary: %#x\n",
		smart->endu_grp_crit_warn_sumry);
	printf("data_units_read				: %'.0Lf\n",
		int128_to_double(smart->data_units_read));
	printf("data_units_written			: %'.0Lf\n",
		int128_to_double(smart->data_units_written));
	printf("host_read_commands			: %'.0Lf\n",
		int128_to_double(smart->host_reads));
	printf("host_write_commands			: %'.0Lf\n",
		int128_to_double(smart->host_writes));
	printf("controller_busy_time			: %'.0Lf\n",
		int128_to_double(smart->ctrl_busy_time));
	printf("power_cycles				: %'.0Lf\n",
		int128_to_double(smart->power_cycles));
	printf("power_on_hours				: %'.0Lf\n",
		int128_to_double(smart->power_on_hours));
	printf("unsafe_shutdowns			: %'.0Lf\n",
		int128_to_double(smart->unsafe_shutdowns));
	printf("media_errors				: %'.0Lf\n",
		int128_to_double(smart->media_errors));
	printf("num_err_log_entries			: %'.0Lf\n",
		int128_to_double(smart->num_err_log_entries));
	// printf("Warning Temperature Time		: %u\n",
	// 	le32_to_cpu(smart->warning_temp_time));
	// printf("Critical Composite Temperature Time	: %u\n",
	// 	le32_to_cpu(smart->critical_comp_time));
	// for (i = 0; i < 8; i++) {
	// 	__s32 temp = le16_to_cpu(smart->temp_sensor[i]);

	// 	if (temp == 0)
	// 		continue;
	// 	printf("Temperature Sensor %d           : %d C\n", i + 1,
	// 		temp - 273);
	// }
	// printf("Thermal Management T1 Trans Count	: %u\n",
	// 	le32_to_cpu(smart->thm_temp1_trans_count));
	// printf("Thermal Management T2 Trans Count	: %u\n",
	// 	le32_to_cpu(smart->thm_temp2_trans_count));
	// printf("Thermal Management T1 Total Time	: %u\n",
	// 	le32_to_cpu(smart->thm_temp1_total_time));
	// printf("Thermal Management T2 Total Time	: %u\n",
	// 	le32_to_cpu(smart->thm_temp2_total_time));
}


int nvme_submit_admin_passthru(int fd, struct nvme_passthru_cmd *cmd)
{
	return ioctl(fd, NVME_IOCTL_ADMIN_CMD, cmd);
}


int nvme_get_log14(int fd, __u32 nsid, __u8 log_id, __u8 lsp, __u64 lpo,
                 __u16 lsi, bool rae, __u8 uuid_ix, __u32 data_len, void *data)
{
	__u32 numd = (data_len >> 2) - 1;
	__u16 numdu = numd >> 16, numdl = numd & 0xffff;
	__u32 cdw10 = log_id | (numdl << 16) | (rae ? 1 << 15 : 0) | lsp << 8;

	struct nvme_admin_cmd cmd = {
		.opcode		= nvme_admin_get_log_page,
		.nsid		= nsid,
		.addr		= (__u64)(uintptr_t) data,
		.data_len	= data_len,
		.cdw10		= cdw10,
		.cdw11		= numdu | (lsi << 16),
		.cdw12		= lpo & 0xffffffff,
		.cdw13		= lpo >> 32,
		.cdw14		= uuid_ix,
	};

	return nvme_submit_admin_passthru(fd, &cmd);
}

int nvme_get_log13(int fd, __u32 nsid, __u8 log_id, __u8 lsp,
		 __u64 lpo, __u16 lsi, bool rae, __u32 data_len,
		 void *data)
{
	return nvme_get_log14(fd, nsid, log_id, lsp, lpo, lsi, rae, 0,
			      data_len, data);
}

int nvme_get_log(int fd, __u32 nsid, __u8 log_id, bool rae,
		 __u32 data_len, void *data)
{
	__u32 offset = 0, xfer_len = data_len;
	void *ptr = data;
	int ret;

	/*
	 * 4k is the smallest possible transfer unit, so by
	 * restricting ourselves for 4k transfers we avoid having
	 * to check the MDTS value of the controller.
	 */
	do {
		xfer_len = data_len - offset;
		if (xfer_len > 4096)
			xfer_len = 4096;

		ret = nvme_get_log13(fd, nsid, log_id, NVME_NO_LOG_LSP,
				     offset, 0, rae, xfer_len, ptr);
		if (ret)
			return ret;

		offset += xfer_len;
		ptr += xfer_len;
	} while (offset < data_len);

	return 0;
}


int nvme_smart_log(int fd, __u32 nsid, struct nvme_smart_log *smart_log)
{
	return nvme_get_log(fd, nsid, NVME_LOG_SMART, false,
			sizeof(*smart_log), smart_log);
}


int main(int argc, char **argv){

	struct nvme_smart_log smart_log;
	enum nvme_print_flags flags;
	int err, fd;

	struct config {
		__u32 namespace_id;
		int   raw_binary;
		char *output_format;
		int   human_readable;
	};

	struct config cfg = {
		.namespace_id = NVME_NSID_ALL,
		.output_format = "normal",
	};
    printf("%s\n",argv[1]);
    fd = opendev(argv[1], 0, OPENDEV_PART, NULL);
	if(fd == -1){
		perror("open");
		printf("errno: %d\n",errno);
		return 1;
	}
    printf("fd: %d\n",fd);
    err = nvme_smart_log(fd, cfg.namespace_id, &smart_log);
    nvme_show_smart_log(&smart_log, cfg.namespace_id, argv[1],
				    flags);

	close(fd);
    return 0;

    }
