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

void CachedDevice::bch_wirteback_add(unsigned sectors)
{
    if (has_dirty_ == false) {
        return;
    }

    has_dirty_ = true;

    if (
}

void CachedDevice::request_write(Bio *bio)
{
    if (should_writeback(bio)) {
       bch_writeback_add(bio->get_sectors()); 
    } else {

    }
}

class RequestQueue {


    void cached_dev_make_request(Bio *bio);

};

void RequestQueue::cached_dev_make_request(Bio *bio)
{
    if (bio->has_data()) {
        if (bio->is_write()) {

        }
    }
}
