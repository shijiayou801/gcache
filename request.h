#include <vector>
#include <stdint.h>


struct Object {

};

struct BioVec {
    struct Object *bv_obj;
    unsigned int bv_len;
    unsigned int bv_offset;
};

enum BioOpType {
   BIO_OP_WRITE = 0,
};

class Bio {
  public:
    bool has_data() const {
        return !bi_io_vec_.empty();
    }

    bool is_write() const {
        return op_type_ == BIO_OP_WRITE;
    }

    bool is_sync_req() const {
        return sync_;
    }

    unsigned int get_sectors() {
        return num_sectors_;
    }

  private:
    unsigned int num_sectors_;

    std::vector<BioVec> bi_io_vec_;

    BioOpType op_type_;

    bool sync_;
};

struct GcStat {
    unsigned in_use;  // Percentage of TODO? when is it updated

};


class CacheSet {
  public:
    uint32_t get_gc_in_use() const {
        return gc_stats_.in_use;
    }

  private:
    struct GcStat gc_stats_;
};


class BcacheDevice {
  public:
    uint32_t get_gc_in_use() const {
        return cache_set_.get_gc_in_use();
    }

  private:
    CacheSet cache_set_;

};

enum CacheMode {
    // Write to cache and backing store
    CACHE_MODE_WRITETHROUGH = 0,

    // Write only to cache and sync to backing store at a later time
    CACHE_MODE_WRITEBACK = 1,

    // Write only to backing store TODO
    CACHE_MODE_WRITEAROUND = 2,

    // TODO
    CACHE_MODE_NONE = 3,
};

class CachedDevice {
  public:
    void request_write(Bio *bio);

  private:
    bool should_writeback(Bio *bio) const;

    void bch_wirteback_add(unsigned sectors);

    BcacheDevice disk_;

    CacheMode cache_mode_;

    bool has_dirty_;
};

const uint32_t CUTOFF_WRITEBACK_SYNC = 75;
const uint32_t CUTOFF_WRITEBACK = 50;

bool CachedDevice::should_writeback(Bio *bio) const
{
    unsigned threshold = bio->is_sync_req()
        ? CUTOFF_WRITEBACK_SYNC
        : CUTOFF_WRITEBACK;
    
    if (cache_mode_ != CACHE_MODE_WRITEBACK) {
       return false;
    }

    // If in use is over the threshold, then we should not
    // writeback
    return disk_.get_gc_in_use() < threshold;
}

void CachedDevice::write_bdev_super()
{

}

void CachedDevice::writeback_queue()
{
    wake_up_process(writeback_thread);
}

void CachedDevice::writeback_add()
{
    if (has_dirty) {
        return;
    }

    has_dirty = true;

    if (sb.state != BDEV_STATE_DIRTY) {
        sb.state = BDEV_STATE_DIRTY;
        write_bdev_super();
    }

    writeback_queue();
}

void CachedDevice::write(CacheIoContext *ioc)
{
    if (should_writeback(bio)) {
        ioc->bypass = false;
        ioc->writeback = true;
    }

    if (ioc->bypass) {

      // Send io to backing store
      generic_make_request(ioc->bio);

      return;
    }

    if (ioc->writeback) {
        bch_writeback_add();
    }
}

void CachedDevice::read(CachedIoContext *ioc)
{
    
}

class RequestQueue {

    void cached_dev_make_request(Bio *bio);

};

struct CacheIoCtx {
    bool bypass;
    bool writeback;
};

void RequestQueue::cached_dev_make_request(Bio *bio)
{
    CachedDevice *cached_dev; // TODO:
    CacheIoCtx *io_ctx = new CacheIoCtx(bio);

    if (bio->has_data()) {
        cached_dev->no_data(io_ctx);
        return;
    }

    io_ctx->bypass = cached_dev->should_bypass(bio);

    if (bio->is_write()) {
        cached_dev->write(io_ctx);
    } else {
        cached_dev->read(io_ctx);
    }
}
