#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <mellanox-decoder.h>

__attribute__ ((noreturn)) __attribute__ ((format (printf, 1, 2)))
static void
fatal_error (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  exit (1);
}


static int
args_to_insn (int argc, char* argv[], struct mellanox_insn *insn)
{
  int i;

  /* Set length to zero so if anything goes wrong we know we'll return a
     sensible answer.  */
  insn->length = 0;

  for (i = 0; i < argc; ++i)
    {
      const char *str = argv[i];
      long int val;

      if (strncmp (str, "0x", 2) == 0)
        str += 2;

      if (strlen (str) != 2)
        return 0;

      val = strtol (str, NULL, 16);
      if (((val >> 8) != 0)
          && ((val >> 8) != -1))
        return 0;

      insn->data[insn->length] = (bfd_byte) val;
      insn->length++;

      if (insn->length > MELLANOX_MAX_INSN_LENGTH)
        return 0;
    }

  return 1;
}

void
print_operand (struct mellanox_operand *operand)
{
  printf ("          Operand type: ");
  switch (operand->type)
    {
    case op_type_none:
      printf ("None");
      break;
    case op_type_core_reg:
      printf ("Core Register");
      break;
    case op_type_imm:
      printf ("Immediate");
      break;
    }
  printf ("\n");

  if (operand->type != op_type_none)
    printf ("         Operand value: %ld (0x%lx)\n",
    operand->operand, operand->operand);
}

int
main (int argc, char* argv[])
{
  struct mellanox_insn insn;
  struct mellanox_insn_decode decode_result;

  if (argc < 2)
    fatal_error ("Not enough command line arguments.\n");
  argc--;
  argv++;

  /* Read bytes off command line and encode them into a buffer.  */
  if (!args_to_insn (argc, argv, &insn))
    fatal_error ("Failed to read instruction bytes from command line.\n");

  /* Call the decode library to decode them.  */
  if (mellanox_decode (&insn, &decode_result) == 0)
    fatal_error ("Failed to decode instruction bytes.\n");

  /* Display results.  */
  printf ("Result of decode:\n");
  printf ("    Disassembly (insn): %s\n", decode_result.inst_disasm);
  printf ("Disassembly (operands): %s\n", decode_result.inst_ops_disasm);
  printf ("    Instruction length: %d bytes\n", decode_result.insn_length);
  printf ("           LIMM length: %d bytes\n", decode_result.limm_length);

  if (decode_result.dst.type != op_type_none)
    {
      printf (" Destination Operand:\n");
      print_operand (&decode_result.dst);
    }

  if (decode_result.src1.type != op_type_none)
    {
      printf ("    Source 1 Operand:\n");
      print_operand (&decode_result.src1);
    }

  if (decode_result.src2.type != op_type_none)
    {
      printf ("    Source 2 Operand:\n");
      print_operand (&decode_result.src2);
    }

  return 0;
}
