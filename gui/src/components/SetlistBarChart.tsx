import {
  BarChart,
  Bar,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ReferenceLine,
  ResponsiveContainer,
  Cell,
} from 'recharts'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import type { SongAnalysis } from '@/lib/store'

interface Props {
  songs: SongAnalysis[]
  medianLufs: number
}

const GRADE_COLORS: Record<string, string> = {
  Good: 'var(--chart-1)',
  Fair: 'var(--chart-4)',
  'Needs Attention': 'var(--destructive)',
}

export default function SetlistBarChart({ songs, medianLufs }: Props) {
  const data = songs.map((song) => ({
    name: song.song_name,
    lufs: song.loudness.integrated_lufs ?? -60,
    grade: song.scores.grade,
  }))

  return (
    <Card>
      <CardHeader className="pb-2">
        <CardTitle className="text-base">Setlist Loudness Comparison</CardTitle>
      </CardHeader>
      <CardContent>
        <ResponsiveContainer width="100%" height={250}>
          <BarChart data={data} margin={{ top: 5, right: 20, bottom: 5, left: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="var(--border)" />
            <XAxis dataKey="name" tick={{ fontSize: 11, fill: 'var(--muted-foreground)' }} />
            <YAxis
              tick={{ fontSize: 11, fill: 'var(--muted-foreground)' }}
              domain={['auto', 'auto']}
              label={{ value: 'LUFS', angle: -90, position: 'insideLeft', style: { fontSize: 11, fill: 'var(--muted-foreground)' } }}
            />
            <Tooltip
              formatter={(v: number) => [v?.toFixed(1) + ' LUFS', 'Integrated']}
              contentStyle={{ backgroundColor: 'var(--popover)', border: '1px solid var(--border)', color: 'var(--popover-foreground)' }}
            />
            {isFinite(medianLufs) && (
              <ReferenceLine
                y={medianLufs}
                stroke="var(--muted-foreground)"
                strokeDasharray="5 5"
                label={{
                  value: `Median: ${medianLufs.toFixed(1)}`,
                  position: 'top',
                  fontSize: 11,
                  fill: 'var(--muted-foreground)',
                }}
              />
            )}
            <Bar dataKey="lufs" radius={[4, 4, 0, 0]}>
              {data.map((entry, idx) => (
                <Cell
                  key={idx}
                  fill={GRADE_COLORS[entry.grade] ?? GRADE_COLORS['Needs Attention']}
                />
              ))}
            </Bar>
          </BarChart>
        </ResponsiveContainer>
      </CardContent>
    </Card>
  )
}
