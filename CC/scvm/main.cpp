#include "svmi.h"

int example_run(int argc, char** argv)
{
  /* load executable image */
  SVMI_image* p_image = new SVMI_image();
  SVMI_STATUS status = p_image->load_from_disk("main_test.vmx");
  if (status != SVMI_STATUS_OK) {
    printf("failed to load image from disk. Status: %d\n", (int)status);
    return 1;
  }

  /* create virtual machine */
  SVMI* p_vm = new SVMI();
  static const SVMI_native_decl native_decls[] = { {
      [](uint8_t* p_pcode, SVMI_context* p_ctx) -> SVMI_STATUS {
      SVMI_VCPU_registers* p_regs = p_ctx->get_regs();
      printf("STRING FROM VM: %s\n", (const char*)p_regs->A);
    }, "print" },
    { nullptr, nullptr }
  };

  status = p_vm->init(p_image, native_decls);
  if (status != SVMI_STATUS_OK) {
    printf("failed to initializing VM. Status: %d\n", (int)status);
    return 1;
  }

  /* create new scope */
  do {
    SVMI_context_scope ctx_scope(*p_vm);
    SVMI_context *p_ctx = ctx_scope.get_context();
    p_ctx->call_push_arg_address(argv);
    p_ctx->call_push_arg_cell(argc);
    p_vm->call(p_ctx, );
  } while (0);
}

int main(int argc, char **argv)
{
  



  return 0;
}