/******************************************************************************/
/* Mednafen NEC PC-FX Emulation Module                                        */
/******************************************************************************/
/* mem-handler.inc:
**  Copyright (C) 2006-2016 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

uint8 MDFN_FASTCALL mem_peekbyte(const v810_timestamp_t timestamp, const uint32 A)
{
 if(A <= 0x007FFFFF)
  return(RAM[A]);
 else if(A <= 0x00FFFFFF)
  return(0xFF);
 else if(A >= 0xF0000000)
  return(BIOSROM[A & 0xFFFFF]);
 else if(FXSCSIROM && A >= 0x80780000 && A <= 0x807FFFFF)
 {
  return(FXSCSIROM[A & 0x7FFFF]);
 }
 return(0xFF);
}

uint16 MDFN_FASTCALL mem_peekhword(const v810_timestamp_t timestamp, const uint32 A) // TODO: Full memory map peeking.
{
 if(A <= 0x007FFFFF)
  return MDFN_de16lsb<true>(&RAM[A]);
 else if(A <= 0x00FFFFFF)
  return 0xFFFF;
 else if(A >= 0xF0000000)
  return MDFN_de16lsb<true>(&BIOSROM[A & 0xFFFFF]);
 else if(FXSCSIROM && A >= 0x80780000 && A <= 0x807FFFFF)
  return MDFN_de16lsb<true>(&FXSCSIROM[A & 0x7FFFF]);

 return 0xFFFF;
}

static uint8 MDFN_FASTCALL mem_rbyte(v810_timestamp_t &timestamp, uint32 A)
{
 if(A <= 0x007FFFFF)
 {
  RAMLPCHECK;
  return(RAM[A]);
 }
 else if(A <= 0x00FFFFFF)
 {
  RAMLPCHECK;
  return(0xFF);
 }
 else if(A >= 0xF0000000)	// BIOS ROM mirrored throughout 0xF0000000-0xFFFFFFFF, the "official" location
				// is at 0xFFF00000(what about on a PC-FXGA??)
 {
  timestamp += 2;
  return(BIOSROM[A & 0xFFFFF]);
 }
 else if(A >= 0xE0000000 && A <= 0xE7FFFFFF && !(A & 1))
 {
  if(BRAMDisabled)
   return(0xFF);

  //printf("%d\n", (A - 0xE0000000) >> 1);
  return(BackupRAM[(A & 0xFFFF) >> 1]);
 }
 else if(A >= 0xE8000000 && A <= 0xE9FFFFFF)
 {
  if(BRAMDisabled)
   return(0xFF);

  if(!(BackupControl & 0x2))
  {
   FXDBG("Read8 from external BRAM when not enabled.");
  }

  return(ExBackupRAM[(A & 0x3FFFF) >> 1]);
 }
 else if(A >= 0x80000000 && A <= 0x807FFFFF)
 {
  //FXDBG("Mem->IO B Read Translation: %08x -> %08x", A, A & 0x7FFFFF);
  return(port_rbyte(timestamp, A & 0x7FFFFF));
 }
 FXDBG("Unknown byte read: %08x", A);
 return(0xFF);
}

static uint16 MDFN_FASTCALL mem_rhword(v810_timestamp_t &timestamp, uint32 A)
{
 if(A <= 0x007FFFFF)
 {
  RAMLPCHECK;
  return MDFN_de16lsb<true>(&RAM[A]);
 }
 else if(A <= 0x00FFFFFF)
 {
  RAMLPCHECK;
  return(0xFFFF);
 }
 else if(A >= 0xF0000000)       // BIOS ROM mirrored throughout 0xF0000000-0xFFFFFFFF, the "official" location
                                // is at 0xFFF00000
 {
  timestamp += 2;
  return MDFN_de16lsb<true>(&BIOSROM[A & 0xFFFFF]);
 }
 else if(A >= 0xA0000000 && A <= 0xA3FFFFFF)
 {
  timestamp += 4;
  return(FXVCE_Read16(0x4));
 }
 else if(A >= 0xA4000000 && A <= 0xA7FFFFFF)
 {
  timestamp += 4;
  return(fx_vdc_chips[0]->Read16(1));
 }
 else if(A >= 0xA8000000 && A <= 0xABFFFFFF)
 {
  timestamp += 4;
  return(fx_vdc_chips[1]->Read16(1));
 }
 else if(A >= 0xAC000000 && A <= 0xAFFFFFFF)
 {
  timestamp += 4;
  return(KING_Read16(timestamp, 0x604));
 }
 else if(A >= 0xB0000000 && A <= 0xBFFFFFFF) // Write only
 {
  return(0);
 }
 else if(A >= 0xE0000000 && A <= 0xE7FFFFFF)
 {
  if(BRAMDisabled)
   return(0xFFFF);

  //printf("%d\n", (A - 0xE0000000) >> 1);
  return(BackupRAM[(A & 0xFFFF) >> 1]);
 }
 else if(A >= 0xE8000000 && A <= 0xE9FFFFFF)
 {
  if(BRAMDisabled)
   return(0xFFFF);

  if(!(BackupControl & 0x2))
  {
   FXDBG("Read16 from external BRAM when not enabled.");
  }

  return(ExBackupRAM[(A & 0x3FFFF) >> 1]);
 }
 else if(A >= 0xF8000000 && A <= 0xFFEFFFFF) // PIO
 {
  FXDBG("PIO H Read: %08x", A);

  return(0x00);
 }
 else if(A >= 0x80000000 && A <= 0x807FFFFF)
 {
  //FXDBG("Mem->IO H Read Translation: %08x -> %08x", A, A & 0x7FFFFF);

  return(port_rhword(timestamp, A & 0x7FFFFF));
 }
 FXDBG("Unknown hword read: %08x", A);

 return(0xFFFF);
}

static uint32 MDFN_FASTCALL mem_rword(v810_timestamp_t &timestamp, uint32 A)
{
 if(A <= 0x007FFFFF)
 {
  RAMLPCHECK;
  return MDFN_de32lsb<true>(&RAM[A]);
 }
 else if(A <= 0x00FFFFFF)
 {
  RAMLPCHECK;
  return(0xFFFFFFFF);
 }
 else if(A >= 0xB0000000 && A <= 0xBFFFFFFF) // Write only
 {
  return(0);
 }
 else
 {
  uint32 ret;

  FXDBG("Warning:  mem_rword() called for 16-bit bus access: %08x", A);

  ret = mem_rhword(timestamp, A);
  ret |= mem_rhword(timestamp, A | 2) << 16;

  return(ret);
 }

 FXDBG("Unknown word read: %08x", A);

 return(0xFFFFFFFF);
}

static void MDFN_FASTCALL mem_wbyte(v810_timestamp_t &timestamp, uint32 A, uint8 V)
{
 if(A <= 0x007FFFFF)
 {
  RAMLPCHECK;
  RAM[A] = V;
 }
 else if(A <= 0x00FFFFFF)
 {
  RAMLPCHECK;
 }
 else if(A >= 0xE0000000 && A <= 0xE7FFFFFF && !(A & 1))
 {
  if(BRAMDisabled)
   return;

  if(BackupControl & 0x1)
  {
   BackupSignalDirty |= (BackupRAM[(A & 0xFFFF) >> 1] != V);
   BackupRAM[(A & 0xFFFF) >> 1] = V;
  }
 }
 else if(A >= 0xE8000000 && A <= 0xE9FFFFFF)
 {
  //printf("ExWrite: %08x", A);
  if(BRAMDisabled)
   return;

  if(BackupControl & 0x2)
  {
   BackupSignalDirty |= (ExBackupRAM[(A & 0x3FFFF) >> 1] != V);
   ExBackupRAM[(A & 0x3FFFF) >> 1] = V;
  }
 }
 else if(A >= 0xF8000000 && A <= 0xFFEFFFFF)
 {
  FXDBG("PIO B Write: %08x %02x", A, V);

  // PIO?
 }
 else if(A >= 0x80000000 && A <= 0x807FFFFF)
 {
  //FXDBG("Mem->IO B Write Translation: %08x %02x -> %08x", A, V, A & 0x7FFFFF);
  port_wbyte(timestamp, A & 0x7FFFFF, V);
 }
}

static void MDFN_FASTCALL mem_whword(v810_timestamp_t &timestamp, uint32 A, uint16 V)
{
 if(A <= 0x007FFFFF)
 {
  RAMLPCHECK;
  MDFN_en16lsb<true>(&RAM[A], V);
 }
 else if(A <= 0x00FFFFFF)
 {
  RAMLPCHECK;
 }
 else if(A >= 0xE0000000 && A <= 0xE7FFFFFF)
 {
  if(BRAMDisabled)
   return;

  if(BackupControl & 0x1)
  {
   BackupSignalDirty |= (BackupRAM[(A & 0xFFFF) >> 1] != (uint8)V);
   BackupRAM[(A & 0xFFFF) >> 1] = (uint8)V;
  }
 }
 else if(A >= 0xE8000000 && A <= 0xE9FFFFFF)
 {
  //printf("ExWrite16: %08x", A);
  if(BRAMDisabled)
   return;

  if(BackupControl & 0x2)
  {
   BackupSignalDirty |= (ExBackupRAM[(A & 0x3FFFF) >> 1] != (uint8)V);
   ExBackupRAM[(A & 0x3FFFF) >> 1] = (uint8)V;
  }
 }
 else if(A >= 0xF8000000 && A <= 0xFFEFFFFF)
 {
  FXDBG("PIO H Write: %08x %04x", A, V);

  // PIO?
 }
 else if(A >= 0xA0000000 && A <= 0xAFFFFFFF) // Read only
 {

 }
 else if(A >= 0xB0000000 && A <= 0xB3FFFFFF)
 {
  timestamp += 2;
  FXVCE_Write16(0x4, V);
 }
 else if(A >= 0xB4000000 && A <= 0xB7FFFFFF)
 {
  timestamp += 2;
  fx_vdc_chips[0]->Write16(1, V);
 }
 else if(A >= 0xB8000000 && A <= 0xBBFFFFFF)
 {
  timestamp += 2;
  fx_vdc_chips[1]->Write16(1, V);
 }
 else if(A >= 0xBC000000 && A <= 0xBFFFFFFF)
 {
  timestamp += 2;
  KING_Write16(timestamp, 0x604, V);
 }
 else if(A >= 0x80000000 && A <= 0x807FFFFF)
 {
  //FXDBG("Mem->IO H Write Translation: %08x %04x -> %08x", A, V, A & 0x7FFFFF);

  port_whword(timestamp, A & 0x7FFFFF, V);
 }
 else
 {
  FXDBG("Unknown hword write: %08x %04x", A, V);
 }
}

static void MDFN_FASTCALL mem_wword(v810_timestamp_t &timestamp, uint32 A, uint32 V)
{
 if(A <= 0x007FFFFF)
 {
  RAMLPCHECK;
  MDFN_en32lsb<true>(&RAM[A], V);
 }
 else if(A <= 0x00FFFFFF)
 {
  RAMLPCHECK;
 }
 else if(A >= 0xA0000000 && A <= 0xAFFFFFFF) // Read only
 {

 }
 else
 {
  FXDBG("Warning:  mem_wword() called for 16-bit bus access: %08x", A);
  mem_whword(timestamp, A, V);
  mem_whword(timestamp, A | 2, V >> 16);
 }
}
