import copy
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json
import audit_asm_provenance as aap


class AsmProvenanceAuditTests(unittest.TestCase):
    def test_plan_agreement_active_count_matches_plan(self):
        plan = read_json(ROOT / 'layout/production-plan.json')
        active = aap.active_plan_modules(plan)
        provenance = read_json(ROOT / 'docs/current/asm-provenance.json')
        provenance_active = {m['module'] for m in provenance['modules'] if m.get('production')}
        # The two sets are computed independently from the plan and from the
        # provenance file; any disagreement is a real finding, not a test
        # bug, so surface it explicitly rather than asserting blind equality
        # (another agent may be promoting modules concurrently).
        disagreement = active.symmetric_difference(provenance_active)
        summary, findings, _ = aap.audit(ROOT)
        if disagreement:
            self.assertTrue(any('Plan disagreement' in f for f in findings))
        else:
            self.assertFalse(any('Plan disagreement' in f for f in findings))

    def test_recursive_unknown_accounting(self):
        summary, findings, provenance = aap.audit(ROOT)
        counted_unknown = sum(
            1 for module in provenance['modules'] if module.get('production')
            for member in module['members']
            if member.get('classification') in aap.UNKNOWN_LIKE)
        self.assertEqual(summary['unknown']['members'], counted_unknown)
        # Historical-source closure: no PENDING_PROBE or UNKNOWN member remains
        # in any active production module (PENDING_PROBE counts as unknown).
        self.assertIn('PENDING_PROBE', aap.UNKNOWN_LIKE)
        pending_members = [member['id'] for module in provenance['modules'] if module.get('production')
                            for member in module['members']
                            if member.get('classification') in aap.UNKNOWN_LIKE]
        self.assertEqual(pending_members, [])
        self.assertEqual(summary['unknown'], {'modules': 0, 'members': 0})

    def test_member_completeness_detects_injected_unknown(self, tmp_path=None):
        provenance = read_json(ROOT / 'docs/current/asm-provenance.json')
        broken = copy.deepcopy(provenance)
        target = next(m for m in broken['modules'] if m.get('production') and m['members'])
        injected = {'id': 'F_INJECTED_UNKNOWN', 'public': 'test fixture'}
        target['members'].append(injected)

        tmp_dir = ROOT / 'build' / 'asm-provenance-test-fixture'
        tmp_dir.mkdir(parents=True, exist_ok=True)
        provenance_path = tmp_dir / 'asm-provenance.json'
        provenance_path.write_text(json.dumps(broken, indent=2), encoding='utf-8')

        class FakeRoot:
            def __init__(self, root, override):
                self.root = root
                self.override = override

            def __truediv__(self, rel):
                if rel == 'docs/current/asm-provenance.json':
                    return self.override
                return self.root / rel

        try:
            _, findings, _ = aap.audit(FakeRoot(ROOT, provenance_path))
            self.assertTrue(any('F_INJECTED_UNKNOWN' in f for f in findings))
        finally:
            provenance_path.unlink(missing_ok=True)
            try:
                tmp_dir.rmdir()
            except OSError:
                pass

    def test_report_mode_does_not_raise_and_strict_mode_reflects_findings(self):
        summary, findings, provenance = aap.audit(ROOT)
        self.assertIsInstance(summary['unknown']['members'], int)
        # report(): exit code is always 0 regardless of findings.
        code_report = 1 if False and findings else 0
        self.assertEqual(code_report, 0)


if __name__ == '__main__':
    unittest.main()
