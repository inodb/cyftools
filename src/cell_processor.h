#pragma once

// DataProcessor.h
#include <string>
#include "cell_header2.h"
#include "cell_row.h"
#include "polygon.h"
#include "cysift.h"
#include <cassert>

#include <cereal/types/vector.hpp>
#include <cereal/archives/portable_binary.hpp>

class CellProcessor {
 public:
  virtual ~CellProcessor() = default;
  
  virtual int ProcessHeader(CellHeader& header) = 0;

  virtual int ProcessLine(Cell& cell) = 0;

  void SetupOutputStream() { 

    // set the output to file or stdout
    if (m_output_file == "-") {
      m_archive = std::make_unique<cereal::PortableBinaryOutputArchive>(std::cout);
    } else {
      m_os = std::make_unique<std::ofstream>(m_output_file, std::ios::binary);
      m_archive = std::make_unique<cereal::PortableBinaryOutputArchive>(*m_os);
    }

    assert(m_archive);
  }
  
  void OutputLine(const Cell& cell) {
    assert(m_archive);
    (*m_archive)(cell);
  }

  void SetCommonParams(const std::string& output_file,
		       const std::string& cmd,
		       bool verbose) {
    m_output_file = output_file;
    m_cmd = cmd;
    m_verbose = verbose;

    
  }
  
protected:

  // copy of the header to manipiulate and output
  CellHeader m_header;

  // name of the output file. empty if none
  std::string m_output_file;

  // string representation of the input command,
  // to track in PG tag in header
  std::string m_cmd;
  
  // common archiver objects
  std::unique_ptr<std::ofstream> m_os;
  std::unique_ptr<cereal::PortableBinaryOutputArchive> m_archive;

  // increase verbosity
  bool m_verbose = false;

  // count the lines as they come, for verbosity
  size_t m_count = 0; 
};

class LineProcessor {
 public:
  virtual ~LineProcessor() = default;
  
  virtual int ProcessHeader(CellHeader& header) = 0;
  
  virtual int ProcessLine(const std::string& line) = 0;
};


// Cut processor
class CutProcessor : public CellProcessor {
  
 public:
  
  void SetParams(const std::unordered_set<std::string>& include) {
    m_include = include;
  }
  
  int ProcessHeader(CellHeader& header) override;
  
  int ProcessLine(Cell& cell) override;
  
 private:
  
  std::unordered_set<std::string> m_include;
  
  std::unordered_set<size_t> m_to_remove;

};

class CleanProcessor : public CellProcessor {

public:

  void SetParams(bool clean_graph, bool clean_meta, bool clean_marker) {
    
    m_clean_graph = clean_graph;
    m_clean_meta = clean_meta;
    m_clean_marker = clean_marker;
    
  }
  
  int ProcessHeader(CellHeader& header) override;
  
  int ProcessLine(Cell& cell) override;


private:

  bool m_clean_graph  = false; 
  bool m_clean_meta   = false; 
  bool m_clean_marker = false; 

  std::unordered_set<size_t> m_to_remove;
  
};


// Pheno processor
class PhenoProcessor : public CellProcessor {
  
 public:
  
  void SetParams(PhenoMap p) {
    m_p = p;
  }
  
  int ProcessHeader(CellHeader& header) override;
  
  int ProcessLine(Cell& cell) override;
  
 private:

  // map connecting marker names to indicies
  std::unordered_map<std::string, size_t> m_marker_map;

  // map of all of the gates (string - pair<float,float>)
  PhenoMap m_p;

};

// Count processor
class CountProcessor : public CellProcessor {
  
 public:
  
  void SetParams() {}
  
  int ProcessHeader(CellHeader& header) override;
  
  int ProcessLine(Cell& cell) override;

  void PrintCount();
  
 private:
  size_t m_count = 0;
};


// Select processor
class SelectProcessor : public CellProcessor {
  
 public:

  void SetParams(uint64_t logor, uint64_t logand, bool lognot) {
    m_or = logor;
    m_and = logand;
    m_not = lognot;
  }
  
  int ProcessHeader(CellHeader& header) override;
  
  int ProcessLine(Cell& cell) override;
  
 private:

  // or flags
  uint64_t m_or;

  // and flags
  uint64_t m_and;

  // should we NOT the output
  bool m_not;
  
};


// Log10 processor
class LogProcessor : public CellProcessor {
  
 public:

  void SetParams()  { }
  
  int ProcessHeader(CellHeader& header) override;

  int ProcessLine(Cell& cell) override;
  
 private:

  std::unordered_set<size_t> m_to_log;

  bool m_bool_warning_emitted = false;
  
};

class ROIProcessor : public CellProcessor {

 public:
  
  void SetParams(bool label,
		 const std::vector<Polygon>& rois) {
    m_label = label;
    m_rois = rois;
  }
  
  int ProcessHeader(CellHeader& header) override;

  int ProcessLine(Cell& cell) override;

 private:

  std::vector<Polygon> m_rois;

  bool m_label;

};

class ViewProcessor : public CellProcessor { 

 public:
  
  void SetParams(bool print_header,
		 bool header_only,
		 int round) {
    
    m_print_header = print_header;
    m_header_only = header_only;
    m_round = round;
    
  }
  
  int ProcessHeader(CellHeader& header) override;

  int ProcessLine(Cell& cell) override;
  
 private:

  bool m_header_only;
  
  bool m_print_header;

  // number of integers to round output to
  int m_round;
};

class BuildProcessor : public CellProcessor { 

 public:
  
  void SetParams() {}
  
  int ProcessHeader(CellHeader& header) override;

  int ProcessLine(Cell& cell) override;
  
 private:

};


class CatProcessor : public CellProcessor { 

 public:
  
  void SetParams(int offset,
		 int sample) {
    
    m_offset = offset;
    m_sample = sample;
    
  }

  void SetOffset(size_t offset) { m_offset = offset; }

  void SetSample(size_t sample) { m_sample = sample; }
  
  int ProcessHeader(CellHeader& header) override;

  int ProcessLine(Cell& cell) override;

  size_t GetMaxCellID() const { return m_max_cellid; }
  
private:
  
  size_t m_cellid_index = static_cast<size_t>(-1);
  
  size_t m_offset;
  size_t m_sample;
  
  // the header to compare with (and print if needed)
  CellHeader m_master_header;
  bool m_master_set = false;

  size_t m_max_cellid = 0;

  std::vector<size_t> m_graph_indicies;
  
};

class CerealProcessor : public LineProcessor { 

 public:
  
  void SetParams(const std::string& filename,
		 const std::string& cmd) {
    m_filename = filename;
    m_cmd = cmd;
  }
  
  int ProcessHeader(CellHeader& header) override;

  int ProcessLine(const std::string& line) override;
  //int ProcessLine(const std::string& line) override;

private:

  int cellid;
  std::vector<float> vec1;

  std::string m_cmd;
  std::string m_filename;
  
  CellHeader m_header;
  
  std::unique_ptr<std::ofstream> m_os;
  std::unique_ptr<cereal::PortableBinaryOutputArchive> m_archive;
  

};

class RadialProcessor : public CellProcessor { 

 public:

  void SetParams(const std::vector<uint64_t>& inner, const std::vector<uint64_t>& outer,
		 const std::vector<uint64_t>& logor, const std::vector<uint64_t>& logand,
		 const std::vector<std::string>& label) {

    // check the radial geometry parameters
    assert(inner.size());
    assert(inner.size() == outer.size());
    assert(inner.size() == logor.size());
    assert(inner.size() == logand.size());
    assert(inner.size() == label.size());    

    m_inner = inner;
    m_outer = outer;
    m_logor = logor;
    m_logand = logand;
    m_label = label;
  }
  
  
  int ProcessHeader(CellHeader& header) override;

  int ProcessLine(Cell& cell) override;
  
 private:
  
  std::vector<uint64_t> m_inner, m_outer, m_logor, m_logand;
  std::vector<std::string> m_label;
  
};
