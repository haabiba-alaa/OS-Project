unsigned int kheap_virtual_address(unsigned int physical_address)
{
    uint32 frame_no = physical_address >> 12;
    uint32 offset = physical_address & 0x00000FFF;


    uint32* ptr_page_table=NULL;
    get_page_table(ptr_page_directory,frame_no,&ptr_page_table);

    if (ptr_page_table != NULL) {
        uint32 virtual_address = (ptr_page_table[PTX(frame_no)] << 12) | offset;
        return virtual_address;
    } else {
        return 0;
    }
}
unsigned int kheap_physical_address(unsigned int virtual_address)
{	uint32 va = (uint32)virtual_address;

    uint32* ptr_page_table=NULL;
    get_page_table(ptr_page_directory,va,&ptr_page_table);

    if (ptr_page_table !=NULL){
      uint32 table_entry=ptr_page_table[PTX(va)];
      int frame_no=table_entry>>12;
      uint32 offset= va & 0x00000FFF;
      unsigned int phys_addr=(frame_no << 12) | offset;
      return phys_addr;
    }

    else {
        // No mapping
      return 0;
    }
    return 0;
}
