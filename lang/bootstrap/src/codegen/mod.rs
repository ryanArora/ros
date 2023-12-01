use crate::{arch::Arch, ast::block::Block, syntax_error::SyntaxError};

mod ir;
mod register_allocation;

pub fn get_code(program: &Block, arch: Arch) -> Result<(), SyntaxError> {
    let mut ir = ir::get_ir(program)?;

    // Optimize here...

    let general_purpose_registers_count = match arch {
        Arch::X86_64 => 8,
    };
    register_allocation::spill_extra_virtual_registers(&mut ir, general_purpose_registers_count);

    Ok(())
}
