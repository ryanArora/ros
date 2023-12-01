use core::fmt;

use indexmap::IndexMap;

use crate::codegen::ir::Register;

use super::ir::{IRStatement, IR};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Interval {
    start: usize,
    end: usize,
}
impl fmt::Display for Interval {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "[{}, {}]", self.start, self.end)
    }
}

pub fn spill_extra_virtual_registers(ir: &mut IR, registers_count: u32) {
    let mut available_registers: IndexMap<Register, bool> = IndexMap::new();
    for i in 0..registers_count {
        available_registers.insert(Register(i), true);
    }

    let live_intervals = get_live_intervals(ir);

    #[derive(Debug, Eq)]
    struct ActiveInterval(Register, Interval);
    impl PartialEq for ActiveInterval {
        fn eq(&self, other: &Self) -> bool {
            self.1.end == other.1.end
        }
    }
    impl PartialOrd for ActiveInterval {
        fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
            self.1.end.partial_cmp(&other.1.end)
        }
    }
    impl Ord for ActiveInterval {
        fn cmp(&self, other: &Self) -> std::cmp::Ordering {
            self.1.end.cmp(&other.1.end)
        }
    }

    let mut active_intervals: Vec<ActiveInterval> = vec![];
    let mut registers: IndexMap<Register, Register> = IndexMap::new();

    for (i_register, i) in &live_intervals {
        active_intervals.retain(|ActiveInterval(j_register, j)| {
            if j.end >= i.start {
                return true;
            }

            let register = registers.get(j_register).unwrap().clone();
            available_registers.insert(register, true);

            return false;
        });

        if active_intervals.len() == registers_count.try_into().unwrap() {
            unimplemented!();
        } else {
            // register[i] ← a register removed from pool of available registers
            let available_register = available_registers
                .iter()
                .find(|(_, available)| **available)
                .unwrap()
                .0
                .clone();
            available_registers.insert(available_register.clone(), false);
            registers.insert(i_register.clone(), available_register.clone());

            // add i to active, sorted by increasing end point
            let new_elem = ActiveInterval(i_register.clone(), i.clone());
            let pos = active_intervals
                .binary_search(&new_elem)
                .unwrap_or_else(|e| e);
            active_intervals.insert(pos, new_elem);
        }
    }

    ir.statements = ir
        .statements
        .iter()
        .map(|stmt| match stmt {
            IRStatement::LoadImmediate { rd, imm } => IRStatement::LoadImmediate {
                rd: registers.get(rd).unwrap().clone(),
                imm: imm.clone(),
            },
            IRStatement::Add { rd, rs1, rs2 } => IRStatement::Add {
                rd: registers.get(rd).unwrap().clone(),
                rs1: registers.get(rs1).unwrap().clone(),
                rs2: registers.get(rs2).unwrap().clone(),
            },
            IRStatement::Subtract { rd, rs1, rs2 } => IRStatement::Subtract {
                rd: registers.get(rd).unwrap().clone(),
                rs1: registers.get(rs1).unwrap().clone(),
                rs2: registers.get(rs2).unwrap().clone(),
            },
            IRStatement::And { rd, rs1, rs2 } => IRStatement::And {
                rd: registers.get(rd).unwrap().clone(),
                rs1: registers.get(rs1).unwrap().clone(),
                rs2: registers.get(rs2).unwrap().clone(),
            },
            IRStatement::Or { rd, rs1, rs2 } => IRStatement::Or {
                rd: registers.get(rd).unwrap().clone(),
                rs1: registers.get(rs1).unwrap().clone(),
                rs2: registers.get(rs2).unwrap().clone(),
            },
            IRStatement::Xor { rd, rs1, rs2 } => IRStatement::Xor {
                rd: registers.get(rd).unwrap().clone(),
                rs1: registers.get(rs1).unwrap().clone(),
                rs2: registers.get(rs2).unwrap().clone(),
            },
            IRStatement::LeftShift { rd, rs1, rs2 } => IRStatement::LeftShift {
                rd: registers.get(rd).unwrap().clone(),
                rs1: registers.get(rs1).unwrap().clone(),
                rs2: registers.get(rs2).unwrap().clone(),
            },
            IRStatement::RightShift { rd, rs1, rs2 } => IRStatement::RightShift {
                rd: registers.get(rd).unwrap().clone(),
                rs1: registers.get(rs1).unwrap().clone(),
                rs2: registers.get(rs2).unwrap().clone(),
            },
            IRStatement::Branch { label } => IRStatement::Branch {
                label: label.clone(),
            },
            IRStatement::BranchNotZero { rs1, label } => IRStatement::BranchNotZero {
                rs1: registers.get(rs1).unwrap().clone(),
                label: label.clone(),
            },
            IRStatement::BranchZero { rs1, label } => IRStatement::BranchZero {
                rs1: registers.get(rs1).unwrap().clone(),
                label: label.clone(),
            },
            IRStatement::Label { label } => IRStatement::Label {
                label: label.clone(),
            },
            IRStatement::Push { rs1 } => IRStatement::Push {
                rs1: registers.get(rs1).unwrap().clone(),
            },
            IRStatement::Pop { rs1 } => IRStatement::Pop {
                rs1: registers.get(rs1).unwrap().clone(),
            },
        })
        .collect();
}

fn get_live_intervals(ir: &IR) -> IndexMap<Register, Interval> {
    let mut intervals: IndexMap<Register, Interval> = IndexMap::new();

    for (i, ir_statement) in ir.statements.iter().enumerate() {
        let updated_registers = match ir_statement {
            IRStatement::LoadImmediate { rd, imm: _ } => vec![rd],
            IRStatement::Add { rd, rs1, rs2 } => vec![rd, rs1, rs2],
            IRStatement::Subtract { rd, rs1, rs2 } => vec![rd, rs1, rs2],
            IRStatement::And { rd, rs1, rs2 } => vec![rd, rs1, rs2],
            IRStatement::Or { rd, rs1, rs2 } => vec![rd, rs1, rs2],
            IRStatement::Xor { rd, rs1, rs2 } => vec![rd, rs1, rs2],
            IRStatement::LeftShift { rd, rs1, rs2 } => vec![rd, rs1, rs2],
            IRStatement::RightShift { rd, rs1, rs2 } => vec![rd, rs1, rs2],
            IRStatement::Branch { label: _ } => vec![],
            IRStatement::BranchNotZero { rs1, label: _ } => vec![rs1],
            IRStatement::BranchZero { rs1, label: _ } => vec![rs1],
            IRStatement::Label { label: _ } => vec![],
            IRStatement::Push { rs1 } => vec![rs1],
            IRStatement::Pop { rs1 } => vec![rs1],
        };

        for register in updated_registers {
            match intervals.get_mut(register) {
                None => {
                    intervals.insert(register.clone(), Interval { start: i, end: i });
                }
                Some(interval) => {
                    interval.end = i;
                }
            };
        }
    }

    intervals
}

#[cfg(test)]
mod tests {
    use indexmap::IndexMap;

    use crate::{
        codegen::{self, ir::Register, register_allocation::Interval},
        parser::Parser,
    };

    #[test]
    fn test_spill_extra_virtual_registers() {
        let mut parser = Parser::new("let a = (1 + 2); let b = (3 + 4); let c = a + b;");
        let program = parser.get_ast().unwrap().unwrap();
        let mut ir = codegen::ir::get_ir(&program).unwrap();
        println!("{}", ir);
        codegen::register_allocation::spill_extra_virtual_registers(&mut ir, 8);
        println!("{}", ir);
    }

    #[test]
    fn test_get_live_intervals() {
        let mut parser = Parser::new("let a = 1; let b = a + 1; let c = a;");
        let program = parser.get_ast().unwrap().unwrap();
        let ir = codegen::ir::get_ir(&program).unwrap();
        let live_intervals = codegen::register_allocation::get_live_intervals(&ir);

        let mut expected_live_intervals: IndexMap<Register, Interval> = IndexMap::new();
        expected_live_intervals.insert(Register(1), Interval { start: 0, end: 2 });
        expected_live_intervals.insert(Register(2), Interval { start: 1, end: 2 });
        expected_live_intervals.insert(Register(3), Interval { start: 2, end: 2 });

        assert_eq!(live_intervals, expected_live_intervals);
    }
}
